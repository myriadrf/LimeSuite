#include <assert.h>
#include "FPGA_common.h"
#include "limesuite/LMS7002M.h"
#include <ciso646>
#include "Logger.h"
#include <complex>
#include "LMSBoards.h"
#include "threadHelper.h"
#include "Profiler.h"

#include "TRXLooper.h"

using namespace std;

namespace lime {

using namespace std::chrono;

static constexpr uint16_t defaultSamplesInPkt = 256;

TRXLooper::TRXLooper(FPGA *f, LMS7002M *chip, int id)
    :
    mRxPacketsToBatch(4), mTxPacketsToBatch(4),
    mTxSamplesInPkt(defaultSamplesInPkt), mRxSamplesInPkt(defaultSamplesInPkt),
    mCallback_logMessage(nullptr),
    mStreamEnabled(false)
{
    const int fifoLen = 512;
    mRx.fifo = new PacketsFIFO<SamplesPacketType*>(fifoLen);
    mTx.fifo = new PacketsFIFO<SamplesPacketType*>(fifoLen);

    lms = chip, fpga = f;
    chipId = id;
    mTimestampOffset = 0;
    rxLastTimestamp.store(0, std::memory_order_relaxed);
    terminateRx.store(false, std::memory_order_relaxed);
    terminateTx.store(false, std::memory_order_relaxed);
    rxDataRate_Bps.store(0, std::memory_order_relaxed);
    txDataRate_Bps.store(0, std::memory_order_relaxed);
    mThreadsReady.store(0);
}

TRXLooper::~TRXLooper()
{
    Stop();
    terminateRx.store(true, std::memory_order_relaxed);
    terminateTx.store(true, std::memory_order_relaxed);
    if (txThread.joinable())
        txThread.join();
    if (rxThread.joinable())
        rxThread.join();
}

// float TRXLooper::GetDataRate(bool tx)
// {
//     return tx ? txDataRate_Bps.load() : rxDataRate_Bps.load();
// }

// int TRXLooper::GetStreamSize(bool tx)
// {
//     return samples12InPkt;
// }

uint64_t TRXLooper::GetHardwareTimestamp(void)
{
    return rxLastTimestamp.load(std::memory_order_relaxed) + mTimestampOffset;
}

void TRXLooper::SetHardwareTimestamp(const uint64_t now)
{
    mTimestampOffset = now - rxLastTimestamp.load(std::memory_order_relaxed);
}
/*
void TRXLooper::RstRxIQGen()
{
    uint32_t data[16];
    uint32_t reg20;
    uint32_t reg11C;
    uint32_t reg10C;
    data[0] = (uint32_t(0x0020) << 16);
    dataPort->ReadLMS7002MSPI(data, &reg20, 1, chipId);
    data[0] = (uint32_t(0x010C) << 16);
    dataPort->ReadLMS7002MSPI(data, &reg10C, 1, chipId);
    data[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD;
    dataPort->WriteLMS7002MSPI(data, 1, chipId);
    data[0] = (uint32_t(0x011C) << 16);
    dataPort->ReadLMS7002MSPI(data, &reg11C, 1, chipId);
    data[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD;             //SXR
    data[1] = (1 << 31) | (uint32_t(0x011C) << 16) | (reg11C | 0x10);    //PD_FDIV
    data[2] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFF;             // mac 3 - both channels
    data[3] = (1 << 31) | (uint32_t(0x0124) << 16) | 0x001F;             //direct control of powerdowns
    data[4] = (1 << 31) | (uint32_t(0x010C) << 16) | (reg10C | 0x8);     // PD_QGEN_RFE
    data[5] = (1 << 31) | (uint32_t(0x010C) << 16) | reg10C;             //restore value
    data[6] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD;             //SXR
    data[7] = (1 << 31) | (uint32_t(0x011C) << 16) | reg11C;             //restore value
    data[8] = (1 << 31) | (uint32_t(0x0020) << 16) | reg20;              //restore value
    dataPort->WriteLMS7002MSPI(data, 9, chipId);
}

void TRXLooper::AlignRxTSP()
{
    uint32_t reg20;
    uint32_t regsA[2];
    uint32_t regsB[2];
    //backup values
    {
        const std::vector<uint32_t> bakAddr = { (uint32_t(0x0400) << 16), (uint32_t(0x040C) << 16) };
        uint32_t data = (uint32_t(0x0020) << 16);
        dataPort->ReadLMS7002MSPI(&data, &reg20, 1, chipId);
        data = (uint32_t(0x0020) << 16) | 0xFFFD;
        dataPort->WriteLMS7002MSPI(&data, 1, chipId);
        dataPort->ReadLMS7002MSPI(bakAddr.data(), regsA, bakAddr.size(), chipId);
        data = (uint32_t(0x0020) << 16) | 0xFFFE;
        dataPort->WriteLMS7002MSPI(&data, 1, chipId);
        dataPort->ReadLMS7002MSPI(bakAddr.data(), regsB, bakAddr.size(), chipId);
    }

    //alignment search
    {
        uint32_t dataWr[4];
        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFF;
        dataWr[1] = (1 << 31) | (uint32_t(0x0400) << 16) | 0x8085;
        dataWr[2] = (1 << 31) | (uint32_t(0x040C) << 16) | 0x01FF;
        dataPort->WriteLMS7002MSPI(dataWr, 3, chipId);
        uint32_t* buf = new uint32_t[sizeof(FPGA_DataPacket) / sizeof(uint32_t)];

        fpga->StopStreaming();
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        fpga->WriteRegister(0x0008, 0x0100);
        fpga->WriteRegister(0x0007, 3);

        dataWr[0] = (1 << 31) | (uint32_t(0x0020) << 16) | 0x55FE;
        dataWr[1] = (1 << 31) | (uint32_t(0x0020) << 16) | 0xFFFD;

        for (int i = 0; i < 100; i++)
        {
            dataPort->WriteLMS7002MSPI(&dataWr[0], 2, chipId);
            dataPort->ResetStreamBuffers();
            fpga->StartStreaming();
            if (dataPort->ReceiveData((char*)buf, sizeof(FPGA_DataPacket), chipId, 50) != sizeof(FPGA_DataPacket))
            {
                lime::warning("Channel alignment failed");
                break;
            }
            fpga->StopStreaming();
            dataPort->AbortReading(chipId);
            if (buf[4] == buf[5])
                break;
        }
        delete[] buf;
    }

    //restore values
    {
        uint32_t dataWr[7];
        dataWr[0] = (uint32_t(0x0020) << 16) | 0xFFFD;
        dataWr[1] = (uint32_t(0x0400) << 16) | regsA[0];
        dataWr[2] = (uint32_t(0x040C) << 16) | regsA[1];
        dataWr[3] = (uint32_t(0x0020) << 16) | 0xFFFE;
        dataWr[4] = (uint32_t(0x0400) << 16) | regsB[0];
        dataWr[5] = (uint32_t(0x040C) << 16) | regsB[1];
        dataWr[6] = (uint32_t(0x0020) << 16) | reg20;
        dataPort->WriteLMS7002MSPI(dataWr, 7, chipId);
    }
}

double TRXLooper::GetPhaseOffset(int bin)
{
    int16_t* buf = new int16_t[sizeof(FPGA_DataPacket)/sizeof(int16_t)];

    dataPort->ResetStreamBuffers();
    fpga->StartStreaming();
    if (dataPort->ReceiveData((char*)buf, sizeof(FPGA_DataPacket), chipId, 50)!=sizeof(FPGA_DataPacket))
    {
        lime::warning("Channel alignment failed");
        delete [] buf;
        return -1000;
    }
    fpga->StopStreaming();
    dataPort->AbortReading(chipId);
    //calculate DFT bin of interest and check channel phase difference
    const std::complex<double> iunit(0, 1);
    const double pi = std::acos(-1);
    const int N = 512;
    std::complex<double> xA(0,0);
    std::complex<double> xB(0, 0);
    for (int n = 0; n < N; n++)
    {
        const std::complex<double> xAn(buf[8+4*n], buf[9+4*n]);
        const std::complex<double> xBn(buf[10+4*n],buf[11+4*n]);
        const std::complex<double> mult = std::exp(-2.0*iunit*pi* double(bin)* double(n)/double(N));
        xA += xAn * mult;
        xB += xBn * mult;
    }
    double phaseA = std::arg(xA) * 180.0 / pi;
    double phaseB = std::arg(xB) * 180.0 / pi;
    double phasediff = phaseB - phaseA;
    if (phasediff < -180.0) phasediff +=360.0;
    if (phasediff > 180.0) phasediff -=360.0;
    delete [] buf;
    return phasediff;
}

void TRXLooper::AlignRxRF(bool restoreValues)
{
    uint32_t reg20 = lms->SPI_read(0x20);
    auto regBackup = lms->BackupRegisterMap();
    lms->SPI_write(0x20, 0xFFFF);
    lms->SetDefaults(LMS7002M::RFE);
    lms->SetDefaults(LMS7002M::RBB);
    lms->SetDefaults(LMS7002M::TBB);
    lms->SetDefaults(LMS7002M::TRF);
    lms->SPI_write(0x10C, 0x88C5);
    lms->SPI_write(0x10D, 0x0117);
    lms->SPI_write(0x113, 0x024A);
    lms->SPI_write(0x118, 0x418C);
    lms->SPI_write(0x100, 0x4039);
    lms->SPI_write(0x101, 0x7801);
    lms->SPI_write(0x103, 0x0612);
    lms->SPI_write(0x108, 0x318C);
    lms->SPI_write(0x082, 0x8001);
    lms->SPI_write(0x200, 0x008D);
    lms->SPI_write(0x208, 0x01FB);
    lms->SPI_write(0x400, 0x8081);
    lms->SPI_write(0x40C, 0x01FF);
    lms->SPI_write(0x404, 0x0006);
    lms->LoadDC_REG_IQ(true, 0x3FFF, 0x3FFF);
    double srate = lms->GetSampleRate(false, LMS7002M::ChA);
    lms->SetFrequencySX(false,450e6);
    int dec = lms->Get_SPI_Reg_bits(LMS7_HBD_OVR_RXTSP);
    if (dec > 4) dec = 0;

    double offsets[] = {1.15/60.0, 1.1/40.0, 0.55/20.0, 0.2/10.0, 0.18/5.0};
    double tolerance[] = {0.9, 0.45, 0.25, 0.14, 0.06};
    double offset = offsets[dec]*srate/1e6;
    std::vector<uint32_t>  dataWr;
    dataWr.resize(16);

    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    fpga->WriteRegister(0x0008, 0x0100);
    fpga->WriteRegister(0x0007, 3);
    bool found = false;
    for (int i = 0; i < 200; i++){
        lms->Modify_SPI_Reg_bits(LMS7_PD_FDIV_O_CGEN, 1);
        lms->Modify_SPI_Reg_bits(LMS7_PD_FDIV_O_CGEN, 0);
        AlignRxTSP();

        lms->SetFrequencySX(true, 450e6+srate/16.0);
        double offset1 = GetPhaseOffset(32);
        if (offset1 < -360)
            break;
        lms->SetFrequencySX(true, 450e6+srate/8.0);
        double offset2 = GetPhaseOffset(64);
        if (offset2 < -360)
            break;
        double diff = offset1-offset2;
        if (abs(diff-offset) < tolerance[dec])
        {
            found = true;
            break;
        }
    }
    if (restoreValues)
        lms->RestoreRegisterMap(regBackup);
    if (found)
        AlignQuadrature(restoreValues);
    else
        lime::warning("Channel alignment failed");
    lms->SPI_write(0x20, reg20);
}

void TRXLooper::AlignQuadrature(bool restoreValues)
{
    auto regBackup = lms->BackupRegisterMap();

    lms->SPI_write(0x20, 0xFFFF);
    lms->SetDefaults(LMS7002M::RBB);
    lms->SetDefaults(LMS7002M::TBB);
    lms->SetDefaults(LMS7002M::TRF);
    lms->SPI_write(0x113, 0x0046);
    lms->SPI_write(0x118, 0x418C);
    lms->SPI_write(0x100, 0x4039);
    lms->SPI_write(0x101, 0x7801);
    lms->SPI_write(0x108, 0x318C);
    lms->SPI_write(0x082, 0x8001);
    lms->SPI_write(0x200, 0x008D);
    lms->SPI_write(0x208, 0x01FB);
    lms->SPI_write(0x400, 0x8081);
    lms->SPI_write(0x40C, 0x01FF);
    lms->SPI_write(0x404, 0x0006);
    lms->LoadDC_REG_IQ(true, 0x3FFF, 0x3FFF);
    lms->SPI_write(0x20, 0xFFFE);
    lms->SPI_write(0x105, 0x0006);
    lms->SPI_write(0x100, 0x4038);
    lms->SPI_write(0x113, 0x007F);
    lms->SPI_write(0x119, 0x529B);
    auto val = lms->Get_SPI_Reg_bits(LMS7_SEL_PATH_RFE, true);
    lms->SPI_write(0x10D, val==3 ? 0x18F : val==2 ? 0x117 : 0x08F);
    lms->SPI_write(0x10C, val==2 ? 0x88C5 : 0x88A5);
    lms->SPI_write(0x20, 0xFFFD);
    lms->SPI_write(0x103, val==2 ? 0x612 : 0xA12);
    val = lms->Get_SPI_Reg_bits(LMS7_SEL_PATH_RFE, true);
    lms->SPI_write(0x10D, val==3 ? 0x18F : val==2 ? 0x117 : 0x08F);
    lms->SPI_write(0x10C, val==2 ? 0x88C5 : 0x88A5);
    lms->SPI_write(0x119, 0x5293);
    double srate = lms->GetSampleRate(false, LMS7002M::ChA);
    double freq = lms->GetFrequencySX(false);

    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    fpga->WriteRegister(0x0008, 0x0100);
    fpga->WriteRegister(0x0007, 3);
    lms->SetFrequencySX(true, freq+srate/16.0);
    bool found = false;
    for (int i = 0; i < 100; i++){

        double offset = GetPhaseOffset(32);
        if (offset < -360)
            break;
        if (fabs(offset) <= 90.0)
        {
            found = true;
            break;
        }
        RstRxIQGen();
    }

    if (restoreValues)
        lms->RestoreRegisterMap(regBackup);
    if (!found)
        lime::warning("Channel alignment failed");
}
*/
/*
int TRXLooper::UpdateThreads(bool stopAll)
{
    bool needTx = false;
    bool needRx = false;

    //check which threads are needed
    if (!stopAll)
    {
        for(auto &i : mRxStreams)
            if(i.used && i.IsActive())
            {
                needRx = true;
                break;
            }
        for(auto &i : mTxStreams)
            if(i.used && i.IsActive())
            {
                needTx = true;
                break;
            }
    }

    //stop threads if not needed
    if((!needTx) && txThread.joinable())
    {
        terminateTx.store(true, std::memory_order_relaxed);
        txThread.join();
    }
    if((!needRx) && rxThread.joinable())
    {
        terminateRx.store(true, std::memory_order_relaxed);
        rxThread.join();
    }

    //configure FPGA on first start, or disable FPGA when not streaming
    if((needTx || needRx) && (!txThread.joinable()) && (!rxThread.joinable()))
    {
        ResizeChannelBuffers();
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        bool align = (mRxStreams[0].used && mRxStreams[1].used && (mRxStreams[0].config.align | mRxStreams[1].config.align));
        if (align)
            AlignRxRF(true);
        //enable FPGA streaming
        fpga->StopStreaming();
        fpga->ResetTimestamp();
        rxLastTimestamp.store(0, std::memory_order_relaxed);
        //Clear device stream buffers
        dataPort->ResetStreamBuffers();

        //enable MIMO mode, 12 bit compressed values
        dataLinkFormat = StreamConfig::FMT_INT12;
        //by default use 12 bit compressed, adjust link format for stream

        for(auto &i : mRxStreams)
            if(i.used && i.config.linkFormat != StreamConfig::FMT_INT12)
            {
                dataLinkFormat = StreamConfig::FMT_INT16;
                break;
            }

        for(auto &i : mTxStreams)
            if(i.used && i.config.linkFormat != StreamConfig::FMT_INT12)
            {
                dataLinkFormat = StreamConfig::FMT_INT16;
                break;
            }

        const uint16_t smpl_width = dataLinkFormat == StreamConfig::FMT_INT12 ? 2 : 0;
        uint16_t mode = 0x0100;

        if (lms->Get_SPI_Reg_bits(LMS7param(LML1_SISODDR)))
            mode = 0x0040;
        else if (lms->Get_SPI_Reg_bits(LMS7param(LML1_TRXIQPULSE)))
            mode = 0x0180;

        fpga->WriteRegister(0x0008, mode | smpl_width);

        const uint16_t channelEnables = (mRxStreams[0].used||mTxStreams[0].used) + 2 * (mRxStreams[1].used||mTxStreams[1].used);
        fpga->WriteRegister(0x0007, channelEnables);

        uint32_t reg9 = fpga->ReadRegister(0x0009);
        const uint32_t addr[] = {0x0009, 0x0009};
        const uint32_t data[] = {reg9 | (5 << 1), reg9 & ~(5 << 1)};
        fpga->StartStreaming();
        fpga->WriteRegisters(addr, data, 2);
        if (!align)
            lms->ResetLogicregisters();
    }
    else if(not needTx and not needRx)
    {
        //disable FPGA streaming
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        fpga->StopStreaming();
    }

    //FPGA should be configured and activated, start needed threads
    if(needRx && (!rxThread.joinable()))
    {
        terminateRx.store(false, std::memory_order_relaxed);
        auto RxLoopFunction = std::bind(&TRXLooper::ReceivePacketsLoop, this);
        rxThread = std::thread(RxLoopFunction);
        SetOSThreadPriority(ThreadPriority::NORMAL, ThreadPolicy::REALTIME, &rxThread);
    }
    if(needTx && (!txThread.joinable()))
    {
        fpga->WriteRegister(0xFFFF, 1 << chipId);
        fpga->WriteRegister(0xD, 0); //stop WFM
        terminateTx.store(false, std::memory_order_relaxed);
        auto TxLoopFunction = std::bind(&TRXLooper::TransmitPacketsLoop, this);
        txThread = std::thread(TxLoopFunction);
        SetOSThreadPriority(ThreadPriority::NORMAL, ThreadPolicy::REALTIME, &txThread);
    }
    return 0;
}
*/

// const lime::SDRDevice::StreamConfig& TRXLooper::GetConfig() const
// {
//     return mConfig;
// }

void TRXLooper::Setup(const SDRDevice::StreamConfig &cfg)
{
    if (rxThread.joinable() || txThread.joinable())
        throw std::logic_error("Samples streaming already running");

    bool needTx = cfg.txCount > 0;
    bool needRx = true; // always need Rx to know current timestamps, cfg.rxCount > 0;
    //bool needMIMO = cfg.rxCount > 1 || cfg.txCount > 1; // TODO: what if using only B channel, does it need MIMO configuration?
    uint8_t channelEnables = 0;

    for (int i = 0; i < cfg.rxCount; ++i) {
        if (cfg.rxChannels[i] > 1)
            throw std::logic_error("Invalid Rx channel, only [0,1] channels supported");
        else
            channelEnables |= (1 << cfg.rxChannels[i]);
    }
    for (int i = 0; i < cfg.txCount; ++i) {
        if (cfg.txChannels[i] > 1)
            throw std::logic_error("Invalid Tx channel, only [0,1] channels supported");
        else
            channelEnables |= (1 << cfg.txChannels[i]);// << 8;
    }
    if ( (cfg.linkFormat != SDRDevice::StreamConfig::I12) && (cfg.linkFormat != SDRDevice::StreamConfig::I16))
        throw std::logic_error("Unsupported stream link format");
    mConfig = cfg;

    //configure FPGA on first start, or disable FPGA when not streaming
    if (!needTx && !needRx)
        return;

    assert(fpga);
    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    rxLastTimestamp.store(0, std::memory_order_relaxed);

    // const uint16_t MIMO_EN = needMIMO << 8;
    // const uint16_t TRIQ_PULSE = lms->Get_SPI_Reg_bits(LMS7param(LML1_TRXIQPULSE)) << 7; // 0-OFF, 1-ON
    // const uint16_t DDR_EN = lms->Get_SPI_Reg_bits(LMS7param(LML1_SISODDR)) << 6; // 0-SDR, 1-DDR
    // const uint16_t MODE = 0 << 5; // 0-TRXIQ, 1-JESD207 (not impelemented)
    // const uint16_t smpl_width =
    //     cfg.linkFormat == SDRDevice::StreamConfig::DataFormat::I12 ? 2 : 0;

    // printf("TRIQ:%i, DDR_EN:%i, MIMO_EN:%i\n", TRIQ_PULSE, DDR_EN, MIMO_EN);
    // const uint16_t reg8 = MIMO_EN | TRIQ_PULSE | DDR_EN | MODE | smpl_width;
    uint16_t mode = 0x0100;
    if (lms->Get_SPI_Reg_bits(LMS7param(LML1_SISODDR)))
        mode = 0x0040;
    else if (lms->Get_SPI_Reg_bits(LMS7param(LML1_TRXIQPULSE)))
        mode = 0x0180;

    const uint16_t smpl_width =
        cfg.linkFormat == SDRDevice::StreamConfig::DataFormat::I12 ? 2 : 0;
    fpga->WriteRegister(0x0008, mode | smpl_width);
    fpga->WriteRegister(0x0007, channelEnables);
    fpga->ResetTimestamp();

    assert(fpga);
    fpga->WriteRegister(0xFFFF, 1 << chipId);
    fpga->StopStreaming();
    fpga->WriteRegister(0xD, 0); //stop WFM
    fpga->ResetTimestamp();

    // Don't just use REALTIME scheduling, or at least be cautious with it.
    // if the thread blocks for too long, Linux can trigger RT throttling
    // which can cause unexpected data packet losses and timing issues.
    // Also need to set policy to default here, because if host process is running
    // with REALTIME policy, these threads would inherit it and exhibit mentioned
    // issues.
    const auto schedulingPolicy = ThreadPolicy::REALTIME;
    if (needRx) {
        terminateRx.store(false, std::memory_order_relaxed);
        auto RxLoopFunction = std::bind(&TRXLooper::ReceivePacketsLoop, this);
        rxThread = std::thread(RxLoopFunction);
        SetOSThreadPriority(ThreadPriority::HIGH, schedulingPolicy, &rxThread);
        pthread_setname_np(rxThread.native_handle(), "lime:RxLoop");

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(2, &cpuset);
        //int rc = pthread_setaffinity_np(rxThread.native_handle(), sizeof(cpu_set_t), &cpuset);
        // if (rc != 0) {
        //   printf("Error calling pthread_setaffinity_np: %i\n", rc);
        // }
    }
    if (needTx) {
        terminateTx.store(false, std::memory_order_relaxed);
        auto TxLoopFunction = std::bind(&TRXLooper::TransmitPacketsLoop, this);
        txThread = std::thread(TxLoopFunction);
        SetOSThreadPriority(ThreadPriority::HIGH, schedulingPolicy, &txThread);
        pthread_setname_np(txThread.native_handle(), "lime:TxLoop");

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(3, &cpuset);
        //int rc = pthread_setaffinity_np(txThread.native_handle(), sizeof(cpu_set_t), &cpuset);
        // if (rc != 0) {
        //   printf("Error calling pthread_setaffinity_np: %i\n", rc);
        // }
    }

    // if (cfg.alignPhase)
    //     TODO: AlignRxRF(true);
    //enable FPGA streaming
}

void TRXLooper::Start()
{
    fpga->StartStreaming();
    {
        std::lock_guard<std::mutex> lock (streamMutex);
        mStreamEnabled = true;
        streamActive.notify_all();
    }
    steamClockStart = steady_clock::now();
    //int64_t startPoint = std::chrono::time_point_cast<std::chrono::microseconds>(pcStreamStart).time_since_epoch().count();
    //printf("Stream%i start %lius\n", chipId, startPoint);
    // if (!mConfig.alignPhase)
    //     lms->ResetLogicregisters();
}

void TRXLooper::Stop()
{
    terminateTx.store(true, std::memory_order_relaxed);
    terminateRx.store(true, std::memory_order_relaxed);
    try {
        if (rxThread.joinable())
            rxThread.join();

        if (txThread.joinable())
            txThread.join();
    } catch (...)
    {
        printf("Failed to join TRXLooper threads\n");
    }
    fpga->StopStreaming();

    RxTeardown();
    TxTeardown();
    // clear memory pools
    mRx.stagingPacket = nullptr;
    mTx.stagingPacket = nullptr;

    delete mRx.memPool;
    mRx.memPool = nullptr;
    delete mTx.memPool;
    mTx.memPool = nullptr;
    mStreamEnabled = false;
}

int TRXLooper::StreamRx(void **dest, uint32_t count, SDRDevice::StreamMeta *meta)
{
    bool timestampSet = false;
    uint32_t samplesProduced = 0;
    const SDRDevice::StreamConfig &config = mConfig;
    const bool useChannelB = config.rxCount > 1;
    //const int totalTimeout = 500;

    lime::complex32f_t *f32_dest[2] = {
        static_cast<lime::complex32f_t *>(dest[0]),
        useChannelB ? static_cast<lime::complex32f_t *>(dest[1]) : nullptr
    };

    bool firstIteration = true;

    //auto start = high_resolution_clock::now();
    while (samplesProduced < count) {
        if(!mRx.stagingPacket && !mRx.fifo->pop(&mRx.stagingPacket, firstIteration, 250))
            return samplesProduced;

        //firstIteration = false;

        if(!timestampSet && meta)
        {
            meta->timestamp = mRx.stagingPacket->timestamp;
            timestampSet = true;
        }

        int expectedCount = count-samplesProduced;
        const int samplesToCopy = std::min(expectedCount, mRx.stagingPacket->size());

        lime::complex32f_t * const *f32_src = reinterpret_cast<lime::complex32f_t * const *>(mRx.stagingPacket->front());

        memcpy(&f32_dest[0][samplesProduced], f32_src[0], samplesToCopy*sizeof(complex32f_t));
        if (useChannelB)
            memcpy(&f32_dest[1][samplesProduced], f32_src[1], samplesToCopy*sizeof(complex32f_t));
        mRx.stagingPacket->pop(samplesToCopy);
        samplesProduced += samplesToCopy;

        if (mRx.stagingPacket->empty())
        {
            mRx.memPool->Free(mRx.stagingPacket);
            mRx.stagingPacket = nullptr;
        }

        // int duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        // if(duration > 300) // TODO: timeout duration in meta
        //     return samplesProduced;
    }
    return samplesProduced;
}

int TRXLooper::StreamTx(const void **samples, uint32_t count, const SDRDevice::StreamMeta *meta)
{
    const SDRDevice::StreamConfig &config = mConfig;
    const bool useChannelB = config.txCount > 1;

    const bool useTimestamp = meta ? meta->useTimestamp : false;
    const bool flush = meta && meta->flush;
    int64_t ts = meta ? meta->timestamp : 0;

    int samplesRemaining = count;
    const lime::complex32f_t* floatSrc[2] = {
        static_cast<const lime::complex32f_t *>(samples[0]),
        useChannelB ? static_cast<const lime::complex32f_t *>(samples[1]) : nullptr
    };

    const int samplesInPkt = mTxSamplesInPkt;
    const int packetsToBatch = mTxPacketsToBatch;
    const int32_t outputPktSize = SamplesPacketType::headerSize
        + packetsToBatch * samplesInPkt
        * (mConfig.format == SDRDevice::StreamConfig::F32 ? sizeof(complex32f_t) : sizeof(complex16_t));

    if (mTx.stagingPacket && mTx.stagingPacket->timestamp + mTx.stagingPacket->size() != meta->timestamp)
    {
        if(!mTx.fifo->push(mTx.stagingPacket))
            return 0;
        mTx.stagingPacket = nullptr;
    }

    while (samplesRemaining) {
        if (!mTx.stagingPacket)
        {
            mTx.stagingPacket = SamplesPacketType::ConstructSamplesPacket(mTx.memPool->Allocate(outputPktSize), samplesInPkt * packetsToBatch, sizeof(complex32f_t));
            if(!mTx.stagingPacket)
                break;
            mTx.stagingPacket->Reset();
            mTx.stagingPacket->timestamp = ts;
            mTx.stagingPacket->useTimestamp = useTimestamp;
        }

        int consumed = mTx.stagingPacket->push(floatSrc, samplesRemaining);
        floatSrc[0] += consumed;
        if(useChannelB)
            floatSrc[1] += consumed;

        samplesRemaining -= consumed;
        ts += consumed;

        if(mTx.stagingPacket->isFull() || flush)
        {
            if(samplesRemaining == 0)
                mTx.stagingPacket->flush = flush;

            if(!mTx.fifo->push(mTx.stagingPacket))
                break;
            mTx.stagingPacket = nullptr;
        }
    }
    return count - samplesRemaining;
}

} // namespace lime
