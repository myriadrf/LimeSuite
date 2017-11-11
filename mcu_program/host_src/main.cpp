#include <stdio.h>
#include "lms7002m_calibrations.h"
#include "lms7002m_controls.h"
#include "lms7002m_filters.h"
#include "spi.h"
#include "mcu.h"
#include "mcuHexBin.h"

#include "LMS7002M.h"
#include "IConnection.h"
#include "ConnectionRegistry.h"
#include <chrono>
#include <fstream>
#include <gnuPlotPipe.h>
using namespace std;

lime::IConnection *serPort = nullptr;
lime::LMS7002M lmsControl;

//use the LMS7002M or calibrate directly from Host
static bool useMCU =0;
static bool tx = 1;
static bool filters = 0;
static float FBW = 5e6;
static bool extLoop = true;
extern float RefClk;

int16_t ReadDCCorrector(bool tx, uint8_t channel)
{
    uint16_t addr = tx ? 0x05C3 : 0x05C7;
    addr += channel;
    uint16_t mask = tx ? 0x03FF : 0x003F;
    uint16_t value = 0;

    SPI_write(addr, 0);
    SPI_write(addr, 0x4000);
    value = SPI_read(addr);
    SPI_write(addr, value & ~0xC000);

    if(value & (mask+1))
        return (value & mask) * -1;
    else
        return (value & mask);
}

int SetExtLoopback(lime::IConnection* port, uint8_t ch, bool enable)
{
    //enable external loopback switches
    const uint32_t LoopbackCtrAddr = 0x0017;
    uint32_t value = 0;
    const uint16_t mask = 0x7;
    const uint8_t shiftCount = (ch==2 ? 4 : 0);
    int status;
    status = port->ReadRegister(LoopbackCtrAddr, value);
    if(status != 0)
        return -1;
    value &= ~(mask << shiftCount);
    value |= enable << shiftCount;   //EN_Loopback
    value |= enable << (shiftCount+1); //EN_Attenuator
    value |= !enable << (shiftCount+2); //EN_Shunt
    status = port->WriteRegister(LoopbackCtrAddr, value);
    if(status != 0)
        return -1;
    return status;
}

void DCIQ()
{
    lime::LMS7002M_RegistersMap *backup;
    int status;
    float freqStart = 2000e6;
    float freqEnd = freqStart;//1000e6;
    float freqStep = 10e6;

    float freq = freqStart;

    vector<float> vfreqs;
    vector<int> vdci, vdcq, vgi, vgq, vph;

    bool isSetBW = false;

    while(freq <= freqEnd)
    {
        SetExtLoopback(lmsControl.GetConnection(), 0, extLoop);
        vfreqs.push_back(freq);
        //status = SetFrequencySX(true, freq + (tx ? 0 : 1e6));
        //status = SetFrequencySX(false, freq + (tx ? -1e6 : 0));
        if(tx)
        {
            //lmsControl.Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 15);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), 1);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), 1);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), 2047);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), 2047);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), 0);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
        }
        else
        {
            lmsControl.Modify_SPI_Reg_bits(LMS7param(DCOFFI_RFE), 0);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(DCOFFQ_RFE), 0);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), 2047);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), 2047);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(IQCORR_RXTSP), 0);
        }
        lmsControl.DownloadAll();
        backup = lmsControl.BackupRegisterMap();

        auto t1 = chrono::high_resolution_clock::now();
        if(useMCU) //using algorithm inside MCU
        {
            if(!isSetBW)
            {
            status = MCU_SetParameter(MCU_REF_CLK, RefClk);
            if(status != 0)
                printf("Failed to set Reference Clk\n");
            status = MCU_SetParameter(MCU_BW, FBW);
            if(status != 0)
                printf("Failed to set Bandwidth\n");
            isSetBW = true;

            }

            if(tx)
                MCU_RunProcedure(1); //initiate Tx calibration
            else
                MCU_RunProcedure(2); //initiate Rx calibration
            status = MCU_WaitForStatus(10000); //wait until MCU finishes
            if(status != 0)
            {
                printf("MCU calibration FAILED : 0x%02X\n", status);
            }

        }
        else //calibrating with PC
        {
            if(tx)
                //status = CalibrateTx();
                status = CalibrateTx(true);
            else
                status = CalibrateRx(true);
        }
        auto t2 = chrono::high_resolution_clock::now();
        long duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

        lmsControl.DownloadAll();
        int16_t dci, dcq, ph, gi, gq;
        if(tx)
        {
            //dci = lmsControl.Get_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP));
            //dcq = lmsControl.Get_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP));
            dci = ReadDCCorrector(true, 0);
            dcq = ReadDCCorrector(true, 1);
            gi = lmsControl.Get_SPI_Reg_bits(LMS7param(GCORRI_TXTSP));
            gq = lmsControl.Get_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP));
            ph = lmsControl.Get_SPI_Reg_bits(LMS7param(IQCORR_TXTSP));

            ph <<= 4;
            ph >>= 4;

            vdci.push_back(dci);
            vdcq.push_back(dcq);
            vgi.push_back(gi);
            vgq.push_back(gq);
            vph.push_back(ph);

            printf("F: %4.0f MHz, DCI:%3i DCQ:%3i GI:%4i GQ:%4i PH:%4i - %s | %li ms\n", freq/1e6, dci, dcq, gi, gq, ph, (status == 0 ? "OK" : "FAIL"), duration);
        }
        else
        {
            //dci = lmsControl.Get_SPI_Reg_bits(LMS7param(DCOFFI_RFE));
            //dcq = lmsControl.Get_SPI_Reg_bits(LMS7param(DCOFFQ_RFE));
            dci = ReadDCCorrector(false, 0);
            dcq = ReadDCCorrector(false, 1);
            gi = lmsControl.Get_SPI_Reg_bits(LMS7param(GCORRI_RXTSP));
            gq = lmsControl.Get_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP));
            ph = lmsControl.Get_SPI_Reg_bits(LMS7param(IQCORR_RXTSP));

            //int16_t rdci = dci;
            //int16_t rdcq = dcq;
            //dci <<= 8;
            /*if(dci & 0x40)
                rdci = -1 * abs(dci & 0x3F);
            if(dcq & 0x40)
                rdcq = -1 * abs(dcq & 0x3F);*/
            ph <<= 4;
            ph >>= 4;

            vdci.push_back(dci);
            vdcq.push_back(dcq);
            vgi.push_back(gi);
            vgq.push_back(gq);
            vph.push_back(ph);

            printf("F: %4.0f MHz, DCI:%3i DCQ:%3i GI:%4i GQ:%4i PH:%4i - %s | %li ms\n", freq/1e6, dci, dcq, gi, gq, ph, (status == 0 ? "OK" : "FAIL"), duration);
        }
        //restore chip state and apply calibration values
        //RestoreRegisterState();
        /*if(!useMCU)
        {
        lmsControl.RestoreRegisterMap(backup);
        lmsControl.UploadAll();
        }*/

        if(tx)
        {
            lmsControl.Modify_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP), dci);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP), dcq);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(GCORRI_TXTSP), gi);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP), gq);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(IQCORR_TXTSP), ph);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
        }
        else
        {
            lmsControl.Modify_SPI_Reg_bits(LMS7param(DCOFFI_RFE), dci);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(DCOFFQ_RFE), dcq);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(GCORRI_RXTSP), gi);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP), gq);
            lmsControl.Modify_SPI_Reg_bits(LMS7param(IQCORR_RXTSP), ph);
        }
        freq += freqStep;
        SetExtLoopback(lmsControl.GetConnection(), 0, false);
    }
    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
        lmsControl.LoadDC_REG_IQ(true, 0x7FFF, 0x7FFF);
    }
}

void Filters()
{
    lime::LMS7002M_RegistersMap *backup;
    int status;
    bool isSetBW = false;

    lmsControl.DownloadAll();
    backup = lmsControl.BackupRegisterMap();
    (void) backup; // backup set but not used, perhaps for side effects ???

    auto t1 = chrono::high_resolution_clock::now();
    if(useMCU) //using algorithm inside MCU
    {
        if(!isSetBW)
        {
        status = MCU_SetParameter(MCU_REF_CLK, RefClk);
        if(status != 0)
            printf("Failed to set Reference Clk\n");
        status = MCU_SetParameter(MCU_BW, FBW);
        if(status != 0)
            printf("Failed to set Bandwidth\n");
        isSetBW = true;

        }

        if(tx)
            MCU_RunProcedure(6); //initiate Tx calibration
        else
            MCU_RunProcedure(5); //initiate Rx calibration
        status = MCU_WaitForStatus(10000); //wait until MCU finishes
        if(status != 0)
        {
            printf("MCU calibration FAILED : 0x%02X\n", status);
        }

    }
    else //calibrating with PC
    {
        if(tx)
            status = TuneTxFilter(FBW);
        else
            status = TuneRxFilter(FBW);
    }
    auto t2 = chrono::high_resolution_clock::now();
    long duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    lmsControl.DownloadAll();
    int16_t rcal_lpflad,ccal_lpflad, lpfh;

    //Restore settings
    int cfb_tia_rfe;
    int ccomp_tia_rfe;
    int rcomp_tia_rfe;
    int rcc_ctl_lpfl_rbb;
    int c_ctl_lpfl_rbb;
    int c_ctl_pga_rbb;
    int rcc_ctl_pga_rbb;
    int rcc_ctl_lpfh_rbb;
    int c_ctl_lpfh_rbb;
    int pd_lpfl_rbb;
    int pd_lpfh_rbb;
    int input_ctl_pga_rbb;

    if(tx)
    {
        rcal_lpflad = lmsControl.Get_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB));
        ccal_lpflad = lmsControl.Get_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB));
        lpfh = lmsControl.Get_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB));

        printf("Tx filter BW: %.0f MHz, RCAL_LPFLAD: %i, CCAL_LPFLAD: %i, RCAL_LPFH: %i  | %li ms\n",
                FBW/1e6, rcal_lpflad, ccal_lpflad, lpfh, duration);
    }
    else
    {
        //Restore settings
        cfb_tia_rfe = lmsControl.Get_SPI_Reg_bits(LMS7param(CFB_TIA_RFE));
        ccomp_tia_rfe = lmsControl.Get_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE));
        rcomp_tia_rfe = lmsControl.Get_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE));
        rcc_ctl_lpfl_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB));
        c_ctl_lpfl_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB));
        c_ctl_pga_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB));
        rcc_ctl_pga_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB));
        rcc_ctl_lpfh_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB));
        c_ctl_lpfh_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB));
        pd_lpfl_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(PD_LPFL_RBB));
        pd_lpfh_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(PD_LPFH_RBB));
        input_ctl_pga_rbb = lmsControl.Get_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB));

    }
    //restore chip state and apply calibration values
    //RestoreRegisterState();
   /* if(!useMCU)
    {
    lmsControl.RestoreRegisterMap(backup);
    lmsControl.UploadAll();
    }*/

    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(RCAL_LPFLAD_TBB), rcal_lpflad);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(CCAL_LPFLAD_TBB), ccal_lpflad);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), lpfh);
    }
    else
    {
        printf("CFB_TIA: %i\n", cfb_tia_rfe);
        printf("CCOMP: %i\n", ccomp_tia_rfe);
        printf("RCOMP: %i\n", rcomp_tia_rfe);
        printf("RCTL_LPFL: %i\n", rcc_ctl_lpfl_rbb);
        printf("RCTL_LPFH: %i\n", rcc_ctl_lpfh_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(CFB_TIA_RFE), cfb_tia_rfe);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(CCOMP_TIA_RFE), ccomp_tia_rfe);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(RCOMP_TIA_RFE), rcomp_tia_rfe);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFL_RBB), rcc_ctl_lpfl_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFL_RBB), c_ctl_lpfl_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(C_CTL_PGA_RBB), c_ctl_pga_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(RCC_CTL_PGA_RBB), rcc_ctl_pga_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(RCC_CTL_LPFH_RBB), rcc_ctl_lpfh_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(C_CTL_LPFH_RBB), c_ctl_lpfh_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_LPFL_RBB), pd_lpfl_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_LPFH_RBB), pd_lpfh_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(INPUT_CTL_PGA_RBB), input_ctl_pga_rbb);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(ICT_LPF_IN_RBB), 12);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(ICT_LPF_OUT_RBB), 12);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(ICT_PGA_OUT_RBB), 20);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(ICT_PGA_IN_RBB), 20);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(R_CTL_LPF_RBB), 16);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(RFB_TIA_RFE), 16);
    }

    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
        lmsControl.LoadDC_REG_IQ(true, 0x7FFF, 0x7FFF);
    }
}

int main(int argc, char** argv)
{
    //connect to first available device
    auto cachedHandles = lime::ConnectionRegistry::findConnections();
    if(cachedHandles.size() > 0)
        serPort = lime::ConnectionRegistry::makeConnection(cachedHandles.at(0));
    if(serPort == nullptr)
        return 0;
    if (serPort != nullptr && !serPort->IsOpen())
    {
        lime::ConnectionRegistry::freeConnection(serPort);
        printf("failed to open LMS7 control device");
    }
    else
    {
        auto info = serPort->GetDeviceInfo();
        printf("Running tests with %s FW:%s HW:%s\n", info.deviceName.c_str(), info.firmwareVersion.c_str(), info.hardwareVersion.c_str());
    }

    lmsControl.SetConnection(serPort);
    //change SPI switch to BB, just in case it was left for MCU
    lmsControl.SPI_write(0x0006, 0);

    RefClk = lmsControl.GetReferenceClk_SX(false);

    //load initial chip config for testing
    string filename;
    /*if(tx)
        filename = "TxTest.ini";
    else
        filename = "RxTest.ini";*/
    //filename = "CalibSetup.ini";
    filename = "TxDCTest.ini";
    if(lmsControl.LoadConfig(filename.c_str()) != 0)
    {
        printf("Failed to load .ini file\n");
        lime::ConnectionRegistry::freeConnection(serPort);
        return -1;
    }
    lmsControl.UploadAll();
    //calibrating A channel
    //lmsControl.SetActiveChannel(lime::LMS7002M::Channel::ChB);

    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    }

    int status;
    //backup chip state

    //lmsControl.Modify_SPI_Reg_bits(LMS7param(MAC), 2);

    status = 0;
    if(useMCU)
    {
        uint8_t mcuImage[8192*2];
        uint16_t imgSize = 0;
        status = MCU_HEX2BIN("calibrationsLMS7_MCU.hex", sizeof(mcuImage), mcuImage, &imgSize);
        if(status != 0)
            return status;

        fstream fout;
        fout.open("mcu_bin.txt", ios::out);
        fout << "static const uint8_t mcuImage[] = {";
        for(size_t i = 0; i < sizeof(mcuImage); ++i)
        {
            if(i % 16 == 0)
                fout << endl;
            char ctemp[40];
            sprintf(ctemp, "0x%02X,", mcuImage[i]);
            fout << ctemp;
        }
        fout << "}\n";
        fout.close();

        status = MCU_UploadProgram(mcuImage, sizeof(mcuImage));
        if(status != 0)
        {
            printf("MCU programming failed\n");
            return -1;
        }
    }

    if(filters)
        Filters();
    else
        DCIQ();
    lime::ConnectionRegistry::freeConnection(serPort);
/*
    GNUPlotPipe plot(true);
    //plot.write("plot '-' u 1:2 w line title 'DC I', '-' u 1:2 w line title 'DC Q', '-' u 1:2 w line title 'Phase'\n");
    plot.write("plot '-' u 1:2 w line title 'DC I', '-' u 1:2 w line title 'DC Q', '-' u 1:2 w line title 'Phase', '-' u 1:2 w line title 'GainI', '-' u 1:2 w line title 'GainQ'\n");

    for(size_t i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vdci[i]);
    }
    plot.write("e\n");
    for(size_t i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vdcq[i]);
    }
    plot.write("e\n");
    for(size_t i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vph[i]);
    }
    plot.write("e\n");
    for(size_t i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vgi[i]);
    }
    plot.write("e\n");
    for(size_t i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vgq[i]);
    }
    plot.write("e\n");
*/

    return 0;
}
