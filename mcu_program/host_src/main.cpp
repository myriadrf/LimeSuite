#include "ConnectionSTREAM.h"

#include <stdio.h>
#include "lms7002m_calibrations.h"
#include "lms7002m_controls.h"
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
bool useMCU = 1;
bool tx = 1;


//potential list of registers that need to be backed up and restored after calibration
const uint16_t backupStateAddr[] = {
0x0020, 0x0082, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008B, 0x00AE,
0x0100, 0x0101, 0x010C, 0x010D, 0x010E, 0x010F, 0x0110, 0x0111, 0x0112, 0x0113,
0x0115, 0x0116, 0x0117, 0x0118, 0x0119, 0x0200, 0x0201, 0x0202, 0x0203, 0x0204, 0x0208, 0x020C,
0x0242, 0x0243, 0x0400, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x040D, 0x040E, 0x0440, 0x0441, 0x0442,
0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C,
0x044D, 0x044E, 0x044F, 0x0450, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456,
0x0457, 0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x045D, 0x045E, 0x045F, 0x0460,
0x0500, 0x0501, 0x0502, 0x0503, 0x0504, 0x0505, 0x0506, 0x0507, 0x0508, 0x0509,
0x050A, 0x050B, 0x050C, 0x050D, 0x050E, 0x050F, 0x0510, 0x0511, 0x0512, 0x0513,
0x0514, 0x0515, 0x0516, 0x0517, 0x0518, 0x0519, 0x051A, 0x051B, 0x051C, 0x051D,
0x051E, 0x051F, 0x0520, 0x0521, 0x0522, 0x0523, 0x0524, 0x0525, 0x0526, 0x0527,
0x0540, 0x0541, 0x0542, 0x0543, 0x0544, 0x0545, 0x0546, 0x0547, 0x0548, 0x0549,
0x054A, 0x054B, 0x054C, 0x054D, 0x054E, 0x054F, 0x0550, 0x0551, 0x0552, 0x0553,
0x0554, 0x0555, 0x0556, 0x0557, 0x0558, 0x0559, 0x055A, 0x055B, 0x055C, 0x055D,
0x055E, 0x055F, 0x0560, 0x0561, 0x0562, 0x0563, 0x0564, 0x0565, 0x0566, 0x0567,
0x0580, 0x0581, 0x0582, 0x0583, 0x0584, 0x0585, 0x0586, 0x0587, 0x0588, 0x0589,
0x058A, 0x058B, 0x058C, 0x058D, 0x058E, 0x058F, 0x0590, 0x0591, 0x0592, 0x0593,
0x0594, 0x0595, 0x0596, 0x0597, 0x0598, 0x0599, 0x059A, 0x059B, 0x059C, 0x059D,
0x059E, 0x059F, 0x05A0, 0x05A1, 0x05A2, 0x05A3, 0x05A4, 0x05A5, 0x05A6, 0x05A7,
};
const uint16_t backupSXAddr[] = {0x011C, 0x011D, 0x011E, 0x011F, 0x0120, 0x0121, 0x0122, 0x0123, 0x0124};
uint16_t backupRegsSXR[sizeof(backupSXAddr) / sizeof(int16_t)];
uint16_t backupRegsSXT[sizeof(backupSXAddr) / sizeof(int16_t)];
uint16_t backupStateValues[sizeof(backupStateAddr)/sizeof(uint16_t)];
uint16_t backup0x0100;
uint16_t backup0x010D;

void BackupRegisterState()
{
    uint8_t macBck = Get_SPI_Reg_bits(0x0020, 1 << 4 | 0);
    SPI_read_batch(backupStateAddr, backupStateValues, sizeof(backupStateAddr) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(0x0020, 1 << 4 | 0, 1);
    SPI_read_batch(backupSXAddr, backupRegsSXR, sizeof(backupRegsSXR) / sizeof(uint16_t));
    //EN_NEXTRX_RFE could be modified in channel A
    backup0x010D = SPI_read(0x010D);
    //EN_NEXTTX_TRF could be modified in channel A
    backup0x0100 = SPI_read(0x0100);
    Modify_SPI_Reg_bits(0x0020, 1 << 4 | 0, 2);
    SPI_read_batch(backupSXAddr, backupRegsSXT, sizeof(backupRegsSXR) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(0x0020, 1 << 4 | 0, macBck);
}

void RestoreRegisterState()
{
    {
    uint8_t macBck = Get_SPI_Reg_bits(0x0020, 1 << 4 | 0);
    SPI_write_batch(backupStateAddr, backupStateValues, sizeof(backupStateAddr) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(0x0020, 1 << 4 | 0, 1);
    SPI_write(0x010D, backup0x010D); //restore EN_NEXTRX_RFE
    SPI_write(0x0100, backup0x0100); //restore EN_NEXTTX_TRF
    SPI_write_batch(backupSXAddr, backupRegsSXR, sizeof(backupRegsSXR) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(0x0020, 1 << 4 | 0, 2);
    SPI_write_batch(backupSXAddr, backupRegsSXT, sizeof(backupRegsSXR) / sizeof(uint16_t));
    Modify_SPI_Reg_bits(0x0020, 1 << 4 | 0, macBck);
    }
    //reset Tx logic registers, fixes interpolator
    {
    uint16_t x0020val = SPI_read(0x0020);
    SPI_write(0x0020, x0020val & ~0xA000);
    SPI_write(0x0020, x0020val);
    }
}

extern uint32_t rssiCounter;
extern uint32_t readCnt;
extern uint32_t writeCnt;
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

    //load initial chip config for testing
    string filename;
    if(tx)
        filename = "TxCalib.ini";
    else
        filename = "RxMIMO_calibration.ini";
    if(lmsControl.LoadConfig(filename.c_str()) != 0)
    {
        printf("Failed to load .ini file\n");
        lime::ConnectionRegistry::freeConnection(serPort);
        return -1;
    }
    lmsControl.UploadAll();
    //calibrating A channel
    lmsControl.SetActiveChannel(lime::LMS7002M::Channel::ChA);

    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
    }

    int status;
    /*
    status = lmsControl.SetFrequencySX(true, 2140e6);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(ISEL_DAC_AFE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 5);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(ICT_IAMP_GG_FRP_TBB), 6);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(SEL_BAND1_TRF), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(SEL_BAND2_TRF), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(RCAL_LPFH_TBB), 0);
    */
    //backup chip state
    lime::LMS7002M_RegistersMap *backup;

    float freq = 0.300e9;
    lmsControl.Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    //lmsControl.Modify_SPI_Reg_bits(LMS7param(CP2_PLL), 0);
    //lmsControl.Modify_SPI_Reg_bits(LMS7param(CP3_PLL), 0);

    lmsControl.Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 10);
    //lmsControl.DownloadAll();
    //lmsControl.Modify_SPI_Reg_bits(LMS7param(ISEL_DAC_AFE), 0);
    //lmsControl.Modify_SPI_Reg_bits(LMS7param(CG_IAMP_TBB), 25);
    //if(status != 0)
      //  printf("PC TUNE ERROR\n");

    status = 0;
    {
        uint8_t mcuImage[8192];
        uint16_t imgSize = 0;
        status = MCU_HEX2BIN("../calibrationsLMS7_MCU.hex", sizeof(mcuImage), mcuImage, &imgSize);
        if(status != 0)
            return status;

        fstream fout;
        fout.open("mcu_bin.txt", ios::out);
        fout << "static const uint8_t mcuImage[] = {";
        for(int i = 0; i < sizeof(mcuImage); ++i)
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

    tx = 1;
    useMCU = 1;
    float freqStart = 1575e6;
    float freqEnd = 1600e6;
    float freqStep = 100e6;

    freq = freqStart;

    vector<float> vfreqs;
    vector<int> vdci, vdcq, vgi, vgq, vph;

    while(freq <= freqEnd)
    {
        vfreqs.push_back(freq);
        status = SetFrequencySX(true, freq + (tx ? 0 : 1e6));
        status = SetFrequencySX(false, freq + (tx ? -1e6 : 0));
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

        if(useMCU) //using algorithm inside MCU
        {
            status = MCU_SetParameter(MCU_REF_CLK, 30.72e6);
            status = MCU_SetParameter(MCU_BW, 20e6);

            //TODO set desired bandwidth parameter
            if(tx)
                MCU_RunProcedure(1); //initiate Tx calibration
            else
                MCU_RunProcedure(2); //initiate Tx calibration
            status = MCU_WaitForStatus(5000); //wait until MCU finishes
            if(status != 0)
            {
                printf("MCU calibration FAILED : 0x%02X\n", status);
            }

        }
        else //calibrating with PC
        {
            if(tx)
                status = CalibrateTx();
            else
                status = CalibrateRx();
        }

        lmsControl.DownloadAll();
        int16_t dci, dcq, ph, gi, gq;
        if(tx)
        {
            dci = lmsControl.Get_SPI_Reg_bits(LMS7param(DCCORRI_TXTSP));
            dcq = lmsControl.Get_SPI_Reg_bits(LMS7param(DCCORRQ_TXTSP));
            gi = lmsControl.Get_SPI_Reg_bits(LMS7param(GCORRI_TXTSP));
            gq = lmsControl.Get_SPI_Reg_bits(LMS7param(GCORRQ_TXTSP));
            ph = lmsControl.Get_SPI_Reg_bits(LMS7param(IQCORR_TXTSP));

            dci <<= 8;
            dci >>= 8;
            dcq <<= 8;
            dcq >>= 8;
            ph <<= 4;
            ph >>= 4;

            vdci.push_back(dci);
            vdcq.push_back(dcq);
            vgi.push_back(gi);
            vgq.push_back(gq);
            vph.push_back(ph);

            printf("F: %4.0f MHz, DCI:%3i DCQ:%3i GI:%4i GQ:%4i PH:%4i - %s\n", freq/1e6, dci, dcq, gi, gq, ph, (status == 0 ? "OK" : "FAIL"));

            /*printf("   | DC  | GAIN | PHASE\n");
            printf("---+-----+------+------\n");
            printf("I: | %3i | %4i | %i\n", dci, gi, ph);
            printf("Q: | %3i | %4i |\n", dcq, gq);*/
        }
        else
        {
            dci = lmsControl.Get_SPI_Reg_bits(LMS7param(DCOFFI_RFE));
            dcq = lmsControl.Get_SPI_Reg_bits(LMS7param(DCOFFQ_RFE));
            gi = lmsControl.Get_SPI_Reg_bits(LMS7param(GCORRI_RXTSP));
            gq = lmsControl.Get_SPI_Reg_bits(LMS7param(GCORRQ_RXTSP));
            ph = lmsControl.Get_SPI_Reg_bits(LMS7param(IQCORR_RXTSP));

            int16_t rdci = dci;
            int16_t rdcq = dcq;
            //dci <<= 8;
            if(dci & 0x40)
                rdci = -1 * abs(dci & 0x3F);
            if(dcq & 0x40)
                rdcq = -1 * abs(dcq & 0x3F);
            ph <<= 4;
            ph >>= 4;

            vdci.push_back(dci);
            vdcq.push_back(dcq);
            vgi.push_back(gi);
            vgq.push_back(gq);
            vph.push_back(ph);

            printf("F: %4.0f MHz, DCI:%3i DCQ:%3i GI:%4i GQ:%4i PH:%4i - %s\n", freq/1e6, dci, dcq, gi, gq, ph, (status == 0 ? "OK" : "FAIL"));

            /*printf("   | DC  | GAIN | PHASE\n");
            printf("---+-----+------+------\n");
            printf("I: | %3i | %4i | %i\n", rdci, gi, ph);
            printf("Q: | %3i | %4i |\n", rdcq, gq);*/
        }
        //restore chip state and apply calibration values
        //RestoreRegisterState();
        lmsControl.RestoreRegisterMap(backup);
        lmsControl.UploadAll();

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
    }
    if(tx)
    {
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 0);
        lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGMODE_TXTSP), 1);
        lmsControl.LoadDC_REG_IQ(true, 0x7FFF, 0x7FFF);
    }
    lime::ConnectionRegistry::freeConnection(serPort);

    /*GNUPlotPipe plot(true);
    plot.write("plot '-' u 1:2 w line title 'DC I', '-' u 1:2 w line title 'DC Q', '-' u 1:2 w line title 'Phase'\n");
    //plot.write("plot '-' u 1:2 w line title 'DC I', '-' u 1:2 w line title 'DC Q', '-' u 1:2 w line title 'Phase', '-' u 1:2 w line title 'GainI', '-' u 1:2 w line title 'GainQ'\n");

    for(int i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vdci[i]);
    }
    plot.write("e\n");
    for(int i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vdcq[i]);
    }
    plot.write("e\n");
    for(int i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vph[i]);
    }
    plot.write("e\n");
    /*for(int i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vgi[i]);
    }
    plot.write("e\n");
    for(int i = 0; i < vfreqs.size(); ++i)
    {
        plot.writef("%f %i\n", vfreqs[i] / 1e6, vgq[i]);
    }
    plot.write("e\n");*/


    return 0;
}
