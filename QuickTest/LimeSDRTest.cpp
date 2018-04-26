#include <chrono>
#include "RF_H.h"
#include "RF_W.h"
#include "Logger.h"
#include "LMS64CProtocol.h"
#include "LimeSDRTest.h"
#include <thread>
#include "kiss_fft.h"
#include "FPGA_Mini.h"
#include <ctime>

using namespace lime;

const std::vector<std::string>testNames =
{"Clock Network Test", "FPGA EEPROM Test", "LMS7002M Test", "RF Loopback Test"};

LimeSDRTest::LimeSDRTest(TestCallback cb)
{
    callback = cb;
    conn = nullptr;
    step = 0;
    running = false;
    lmsControl = nullptr;
    device = nullptr;
}

LimeSDRTest::~LimeSDRTest()
{
    Disconnect();
}

int LimeSDRTest::UpdateStatus(int event, int progress, const char* msg)
{
    if (!callback)
        return 0;
    if (progress >= 0)
        progress = (100*step+progress)/testNames.size();
    return callback(step,event,progress,msg);
}

void LimeSDRTest::Disconnect()
{
    if (device != nullptr)
    {
        delete device;
        device = nullptr;
    }
}

int LimeSDRTest::Connect()
{
    Disconnect();
    auto handles = LMS7_Device::GetDeviceList();

    if (handles.size() < 1)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, "Error: No Devices Connected");
        return -1;
    }
    if (handles.size() > 1)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, "Error: Multiple Devices Connected");
        return -1;
    }
    if (handles[0].name.find("Mini") == std::string::npos)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, "Error: only LimeSDR-Mini is supported");
        return -1;
    }
    device = LMS7_Device::CreateDevice(handles[0]);

    if (device == nullptr)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, "Error: Unable to connect");
        return -1;
    }

    conn = device->GetConnection();
    lmsControl = device->GetLMS();
    std::string str = "->Device: ";
    str += handles[0].serialize();
    UpdateStatus(LMS_TEST_INFO, -1, str.c_str());
    if (str.find("USB 3") == std::string::npos)
    {
        str = "Warning: USB3 not available";
        UpdateStatus(LMS_TEST_INFO, -1, str.c_str());
    }

    UpdateStatus(LMS_TEST_LOGFILE, -1, handles[0].serial.c_str());
    return 0;
}

int LimeSDRTest::TransferLMS64C(unsigned char* packet)
{
    auto stream = dynamic_cast<LMS64CProtocol*>(conn);
    if (stream->Write(packet, 64) != 64)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, "  Error:Write failed");
        return -1;
    }
    memset(packet, 0, 64);
    if (stream->Read(packet, 64, 5000) != 64)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, "  Error: Read failed");
        return -1;
    }

    if (packet[1] != 1)
    {
        std::string str = "Operation failed: error code " + std::to_string(packet[1]);
        UpdateStatus(LMS_TEST_FAIL, -1, str.c_str());
        return -1;
    }
    return 0;
}

int LimeSDRTest::FPGA_EEPROM_Test()
{
    unsigned char buf[64];
    char str[64];
    UpdateStatus(LMS_TEST_INFO, 0, "->Read EEPROM");

    memset(buf, 0, 64);
    buf[0] = CMD_MEMORY_RD;
    buf[2] = 56;
    buf[13] = 7;				//read count
    buf[19] = 3;

    if (TransferLMS64C(buf) != 0)
        return -1;

    std::snprintf(str, sizeof(str), "->Read data: %02X %02X %02X %02X %02X %02X %02X", buf[32], buf[33], buf[34], buf[35], buf[36], buf[37], buf[38]);
    UpdateStatus(LMS_TEST_INFO, 50, str);

    if (buf[32]<17 || buf[35]<17 || buf[33]>11 || buf[36]>11 || buf[34]>31 || buf[37]>31 || buf[34]==0 || buf[37]==0)
        return -1;
    return 0;
}

int LimeSDRTest::LMS7002mTest()
{
    UpdateStatus(LMS_TEST_INFO, 0, "->Perform Registers Test");

    int st = lmsControl->SPI_write(0xA6, 0x0001);
    if (st != 0)  UpdateStatus(LMS_TEST_INFO, -1, "SPI_write() failed");
    st = lmsControl->SPI_write(0x92, 0xFFFF);
    if (st != 0)  UpdateStatus(LMS_TEST_INFO, -1, "SPI_write() failed");
    st = lmsControl->SPI_write(0x93, 0x03FF);
    if (st != 0)  UpdateStatus(LMS_TEST_INFO, -1, "SPI_write() failed");

    if (lmsControl->RegistersTest(nullptr) != 0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, lime::GetLastErrorMessage());
        return -1;
    }

    lmsControl->ResetChip();

    UpdateStatus(LMS_TEST_INFO, 94, "->External Reset line test");
    int status;
    if (lmsControl->SPI_write(0x0020, 0xFFFD) != 0)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, GetLastErrorMessage());
        return -1;
    }

    int val = lmsControl->SPI_read(0x20, true, &status);
    if (status != 0)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, GetLastErrorMessage());
        return -1;
    }

    char str[64];
    std::snprintf(str, sizeof(str), "  Reg 0x20: Write value 0xFFFD, Read value 0x%04X", val);
    UpdateStatus(LMS_TEST_INFO, 97, str);
    if (val != 0xFFFD)
        return -1;

    lmsControl->ResetChip();
    val = lmsControl->SPI_read(0x20, true, &status);
    if (status != 0)
    {
        UpdateStatus(LMS_TEST_FAIL, -1, GetLastErrorMessage());
        return -1;
    }

    std::snprintf(str, sizeof(str), "  Reg 0x20: value after reset 0x0%4X", val);
    UpdateStatus(LMS_TEST_INFO, 100, str);
    if (val != 0xFFFF)
        return -1;
    return 0;
}

int LimeSDRTest::InitFPGATest(unsigned test, double timeout)
{
    unsigned completed;
    uint32_t addr[] = { 0x61, 0x63};
    uint32_t vals[] = { 0x0, 0x0};
    if (conn->WriteRegisters(addr, vals, 2) != 0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, GetLastErrorMessage());
        return -1;
    }
    auto start = std::chrono::steady_clock::now();
    if (conn->WriteRegister(0x61, test) != 0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, GetLastErrorMessage());
        return -1;
    }

    if (timeout < 0)
        return 0;

    while (1)
    {
        if (conn->ReadRegister(0x65, completed) != 0)
        {
            UpdateStatus(LMS_TEST_INFO, -1, GetLastErrorMessage());
            return -1;
        }
        if ((completed & test) == test)
            return completed;

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        if (elapsed_seconds.count() > timeout)
        {
            std::string str = "  Test Timeout. Last result: ";
            str += std::to_string(completed & test);
            UpdateStatus(LMS_TEST_INFO, -1, str.c_str());
            return completed;
        }
    }
}

int LimeSDRTest::GPIFClkTest()
{
    if ((InitFPGATest(0x01, 1.0) & 0x1) != 0x1)
        return -1;

    uint32_t addr[] = { 0x69, 0x69, 0x69 };
    uint32_t vals[3];
    if (conn->ReadRegisters(addr, vals, 3) != 0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, GetLastErrorMessage());
        return -1;
    }
    std::string str = "  Test results: " + std::to_string(vals[0]) + "; " + std::to_string(vals[1]) + "; " + std::to_string(vals[2]);
    if (vals[0] == vals[1] && vals[1] == vals[2])
    {
        str += " - FAILED";
        UpdateStatus(LMS_TEST_INFO, -1, str.c_str());
        return -1;
    }
    str += " - PASSED";
    UpdateStatus(LMS_TEST_INFO, 25, str.c_str());
    return 0;
}

int LimeSDRTest::VCTCXOTest()
{
    uint8_t id = 0;
    double val = 0;
    unsigned count1;
    unsigned count2;
    std::string units = "";
    if (conn->CustomParameterWrite(&id, &val, 1, units) != 0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, GetLastErrorMessage());
        return -1;
    }
    if ((InitFPGATest(0x04, 1.0) & 0x4) != 0x4)
        return -1;

    uint32_t addr[] = { 0x72, 0x73 };
    uint32_t vals[2];
    if (conn->ReadRegisters(addr, vals, 2) != 0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, GetLastErrorMessage());
        return -1;
    }
    count1 = vals[0] + (vals[1] << 16);
    val = 254;
    if (conn->CustomParameterWrite(&id, &val, 1, units) != 0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, GetLastErrorMessage());
        return -1;
    }
    if ((InitFPGATest(0x04, 1.0) & 0x4) != 0x4)
        return -1;

    if (conn->ReadRegisters(addr, vals, 2) != 0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, GetLastErrorMessage());
        return -1;
    }

    count2 = vals[0] + (vals[1] << 16);
    std::string str = "  Results : " + std::to_string(count1) + " (min); " + std::to_string(count2) + " (max)";

    if ((count1 + 50 > count2) || (count1 + 300 < count2))
    {
        str += " - FAILED";
        UpdateStatus(LMS_TEST_INFO, 75, str.c_str());
        return -1;
    }
    str += " - PASSED";
    UpdateStatus(LMS_TEST_INFO, 75, str.c_str());
    return 0;
}

int LimeSDRTest::ClockNetworkTest()
{
    int ret = 0;
    UpdateStatus(LMS_TEST_INFO, 0, "->REF clock test");
    if (GPIFClkTest() == -1)
    {
        UpdateStatus(LMS_TEST_INFO, 25, "->REF clock test FAILED");
        ret = -1;
    }

    UpdateStatus(LMS_TEST_INFO, 50, "->VCTCXO test");
    if (VCTCXOTest() == -1)
    {
        UpdateStatus(LMS_TEST_INFO, 75, "  FAILED");
        ret = -1;
    }
    return ret;
}

int LimeSDRTest::ConfigureLMS(const std::vector<uint32_t> &lmsConfigRF)
{
    if (conn->IsOpen() == true)   // LMS is connected
    {
        if (lmsControl->ResetChip()!=0)    // reset chip
        {
            UpdateStatus(LMS_TEST_INFO, -1, lime::GetLastErrorMessage());
            return -1;
        }

        for (int i = 0; i < lmsConfigRF.size(); i++)
            lmsControl->SPI_write(lmsConfigRF[i]>>16, lmsConfigRF[i]&0xFFFF,true);

        int decimation = lmsControl->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        int interpolation = lmsControl->Get_SPI_Reg_bits(LMS7param(HBI_OVR_TXTSP));
        // set interface frequency
        if (lmsControl->SetInterfaceFrequency(lmsControl->GetFrequencyCGEN(), interpolation, decimation) != 0)
        {
            UpdateStatus(LMS_TEST_INFO, -1, lime::GetLastErrorMessage());
            return -1;
        }
        //if decimation/interpolation is 0(2^1) or 7(bypass), interface clocks should not be divided
        float interfaceRx_Hz = lmsControl->GetReferenceClk_TSP(LMS7002M::Rx);
        if (decimation != 7)
            interfaceRx_Hz /= pow(2.0, decimation);
        float interfaceTx_Hz = lmsControl->GetReferenceClk_TSP(LMS7002M::Tx);
        if (interpolation != 7)
            interfaceTx_Hz /= pow(2.0, interpolation);

        auto fpga = device->GetFPGA();
        fpga->SetConnection(conn);
        fpga->SetInterfaceFreq(interfaceTx_Hz, interfaceRx_Hz,0);
        // get sampling rate
        decimation = lmsControl->Get_SPI_Reg_bits(LMS7param(HBD_OVR_RXTSP));
        float samplingFreq = lmsControl->GetReferenceClk_TSP(lime::LMS7002M::Rx);
        if (decimation != 7)
                samplingFreq /= pow(2.0, decimation + 1);
        this->nyquistFrequency = samplingFreq;

        return 0;
    }
    return -1;
}

int LimeSDRTest::Reg_write(uint16_t address, uint16_t data)
{
    if (conn == nullptr) return -1;
    int status;
    status = conn->WriteRegister(address, data);
    return status == 0 ? 0 : -1;
}

uint16_t LimeSDRTest::Reg_read(uint16_t address)
{
    if (conn == nullptr) return 0;
    uint32_t dataRd = 0;
    int status;
    status = conn->ReadRegister(address, dataRd);
    if (status == 0)
        return dataRd & 0xFFFF;
    else
        return 0;
}

int LimeSDRTest::CollectAndCalculateFFT(float* results, unsigned fftSize)
{
    const unsigned int buffer_size = 4096 * 16; // samples from LMS buffer size
    long bytesToRead = buffer_size;				// reading blocks size
    char data_buffer[buffer_size] = {0};
    long totalBytesReceived = 0;				// set total received byte counter to 0

    //switch off Rx/Tx
    uint16_t interface_ctrl_000A = Reg_read(0x000A);
    Reg_write(0x000A, interface_ctrl_000A & ~0x3);

    //reset hardware timestamp to 0
    uint16_t interface_ctrl_0009 = Reg_read(0x0009);
    Reg_write(0x0009, interface_ctrl_0009 & ~0x3);
    Reg_write(0x0009, interface_ctrl_0009 | 0x3);
    Reg_write(0x0009, interface_ctrl_0009 & ~0x3);

    const uint16_t MIMO_INT_EN = 1 << 8;
    Reg_write(0x0007, 1);
    Reg_write(0x0008, MIMO_INT_EN | 0x2);

    //switch on Rx
    interface_ctrl_000A = Reg_read(0x000A);
    Reg_write(0x000A, interface_ctrl_000A | 0x1);

    //Receive samples
    conn->ResetStreamBuffers();
    int handle = conn->BeginDataReading(data_buffer, bytesToRead,0);
    if (conn->WaitForReading(handle, 1000) == false)
    {
        UpdateStatus(LMS_TEST_INFO, -1, "  Failed to read samples: Timeout");
        return -1;
    }
    if (( totalBytesReceived =  conn->FinishDataReading(data_buffer, bytesToRead, handle))<=0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, "  Failed to read samples: underrun");
        return -1;
    }

    //stop Tx Rx if they were active
    interface_ctrl_000A = Reg_read(0x000A);
    Reg_write(0x000A, interface_ctrl_000A & ~0x3);

    //parse received samples and store them for FFT
    if (totalBytesReceived == 0) {
        UpdateStatus(LMS_TEST_INFO, -1, "  Failed to read samples: No samples received");
        return -1;
    }

    kiss_fft_cpx* m_fftCalcOut = new kiss_fft_cpx[fftSize];
    kiss_fft_cpx* m_fftCalcIn = new kiss_fft_cpx[fftSize];

    int iSamplesCollected = 0;    // samples colleced
    int qSamplesCollected = 0;

    char* bufStart = data_buffer; // pointer buffer
    short sample;                // temp variable for parsing

    //Parse samples from packet
    for (int b = 0; b < totalBytesReceived; b += 3)
    {
        if ((b%4080) == 0)		//skip packet headers
            bufStart += 16;
        sample = (bufStart[b + 1 + 3] & 0x0F) << 8;
        sample |= (bufStart[b + 3] & 0xFF);
        sample = sample << 4;
        sample = sample >> 4;
        m_fftCalcIn[iSamplesCollected++].r = (float)sample;

        sample = bufStart[b + 2 + 3] << 4;
        sample |= (bufStart[b + 1 + 3] >> 4) & 0x0F;
        sample = sample << 4;
        sample = sample >> 4;
        m_fftCalcIn[qSamplesCollected++].i = (float)sample;
        if (qSamplesCollected == fftSize)
            break;
    }

    // allocate buffer for FFT
    kiss_fft_cfg m_kissBuffer = kiss_fft_alloc(fftSize, 0, NULL, NULL);
    // FFT calculation
    kiss_fft(m_kissBuffer, m_fftCalcIn, m_fftCalcOut);
    //Apply normalization and store the results
    int output_index = 0;
    for (int i = 0; i<fftSize; ++i)
    {
        output_index = i + fftSize / 2 - 1;
        if (output_index >= fftSize)
            output_index -= fftSize;
        results[output_index] = 20* log10(sqrt(m_fftCalcOut[i].r*m_fftCalcOut[i].r + m_fftCalcOut[i].i*m_fftCalcOut[i].i)/(fftSize*2047.0));
    }

    free(m_kissBuffer);
    delete[] m_fftCalcOut;
    delete[] m_fftCalcIn;
    return 0;
}

bool LimeSDRTest::RunTest(RFTestData &data)
{
    const double rfTestTolerance_dB = 5.0;
    const double rfTestTolerance_Hz = 50e3;
    // set rx and tx freq.
    if (lmsControl->SetFrequencySX(lime::LMS7002M::Tx, data.txfreq)!=0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, lime::GetLastErrorMessage());
        return false;
    }

    if (lmsControl->SetFrequencySX(lime::LMS7002M::Rx, data.rxfreq)!=0)
    {
        UpdateStatus(LMS_TEST_INFO, -1, lime::GetLastErrorMessage());
        return false;
    }
    lmsControl->Modify_SPI_Reg_bits(LMS7_MAC, 1);
    lmsControl->Modify_SPI_Reg_bits(LMS7_LOSS_MAIN_TXPAD_TRF, data.padGain);
    lmsControl->Modify_SPI_Reg_bits(LMS7_LOSS_LIN_TXPAD_TRF, data.padGain);
    lmsControl->LoadDC_REG_IQ(true, 0x7000, 0x7000);

    const unsigned fftSize = 1024 * 16;
    float peakAmplitude, peakFrequency;
    bool passed;

    for (int retry = 0; retry < 3; retry++)
    {
        peakAmplitude = -1000;
        peakFrequency = 0;
        float output[fftSize];
        passed = true;
        if (CollectAndCalculateFFT(output, fftSize) == 0)
        {
            for (int i = 0; i<fftSize; ++i)
                if (output[i] > peakAmplitude)
                {
                    peakAmplitude = output[i];
                    peakFrequency = (i + 1)*(this->nyquistFrequency / fftSize);
                    peakFrequency -= this->nyquistFrequency / 2;
                }
        }
        if ((peakAmplitude < data.peakval - rfTestTolerance_dB) || (peakAmplitude > data.peakval + rfTestTolerance_dB))
            passed = false;

        if ((peakFrequency > data.peakfreq+ rfTestTolerance_Hz) || (peakFrequency < data.peakfreq - rfTestTolerance_Hz))
            passed = false;
        if (passed)
            break;
    }

    data.peakval = peakAmplitude;
    data.peakfreq = peakFrequency;

    return passed;
}

int LimeSDRTest::RFTest()
{
    const uint16_t LoopbackCtrAddr = 0x0017;

    RFTestData rfBoardLNAW = {1000e6, 1005e6, -12.0, 5e6, 0};
    RFTestData rfBoardLNAH = {2100e6, 2105e6, -12.0, 5e6, 0};

    UpdateStatus(LMS_TEST_INFO, 0, "->Configure LMS");
    if (ConfigureLMS(lmsCfgExtW) != 0)
    {
        UpdateStatus(LMS_TEST_FAIL, 0, "Failed to configure LMS");
        return -1;
    }

    UpdateStatus(LMS_TEST_INFO, 10, "->Run Tests (TX_2 -> LNA_W):");

    if (conn->WriteRegister(LoopbackCtrAddr, 0x1100) != 0)			//enable loopback
    {
        UpdateStatus(LMS_TEST_FAIL, -1, lime::GetLastErrorMessage());
        return -1;
    }

    bool passed = true;

    bool testPassed = RunTest(rfBoardLNAW);
    std::string str = RFTestInfo(rfBoardLNAW, testPassed);
    UpdateStatus(LMS_TEST_INFO, 50, str.c_str());
    if (testPassed == false)
        passed = false;

    UpdateStatus(LMS_TEST_INFO, 50, "->Configure LMS");
    if (ConfigureLMS(lmsCfgExtH) != 0)
    {
        UpdateStatus(LMS_TEST_FAIL, 50, "Failed to configure LMS");
        return -1;
    }

    UpdateStatus(LMS_TEST_INFO, 60, "->Run Tests (TX_1 -> LNA_H):");

    if (conn->WriteRegister(LoopbackCtrAddr, 0x2200) != 0)			//enable loopback
    {
        UpdateStatus(LMS_TEST_FAIL, -1, lime::GetLastErrorMessage());
        return -1;
    }

    testPassed = RunTest(rfBoardLNAH);
    str = RFTestInfo(rfBoardLNAH, testPassed);
    UpdateStatus(LMS_TEST_INFO, 100, str.c_str());
    if (testPassed == false)
        passed = false;

    if (passed)
        return 0;
    return -1;
}

std::string LimeSDRTest::RFTestInfo(const RFTestData data, bool passed)
{
    char buffer[128];
    if (data.peakval < -999)
        snprintf(buffer, sizeof(buffer)-1, "  (SXR=%3.1fMHz, SXT=%3.1fMHz) - ", data.rxfreq / 1e6, data.txfreq / 1e6);
    else
    snprintf(buffer, sizeof(buffer)-1, "  (SXR=%3.1fMHz, SXT=%3.1fMHz, TXPAD=%d): Result:(%3.1f dBFS, %3.2f MHz) - ", data.rxfreq/1e6, data.txfreq/1e6,data.padGain,data.peakval,data.peakfreq/1e6);
    std::string str = buffer;
    str += passed == true ? "PASSED" : "FAILED";
    return str;
}

void LimeSDRTest::_perform_tests(LimeSDRTest* pthis)
{
    int status = 0;

    while (pthis->running.load() == true)
    {
        int ret = 0;
        std::string str;
        if (pthis->step < testNames.size())
        {
            str = "\n[ " + testNames[pthis->step] + " ]";
            pthis->UpdateStatus(LMS_TEST_INFO, 0, str.c_str());
        }
        else
        {
            pthis->callback(-1, status == 0 ? LMS_TEST_SUCCESS : LMS_TEST_FAIL, 100, status == 0 ? "\n=> Board tests PASSED <=\n" : "\n=> Board tests FAILED <=\n");
            pthis->running.store(false);
            pthis->Disconnect();
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> duration = end - pthis->tp_start;
            char str[64];
            std::snprintf(str, sizeof(str), "Elapsed time: %1.2f seconds\n",duration.count());
            pthis->callback(-1,  LMS_TEST_INFO, 100, str);
            return;
        }

        switch (pthis->step)
        {
            case 0: ret = pthis->ClockNetworkTest(); break;
            case 1: ret = pthis->FPGA_EEPROM_Test(); break;
            case 2: ret = pthis->LMS7002mTest(); break;
            case 3: ret = pthis->RFTest(); break;
        }

        str = "->" + testNames[pthis->step];
        str += (ret == 0) ? " PASSED" : " FAILED";
        pthis->UpdateStatus((ret == 0) ? LMS_TEST_SUCCESS : LMS_TEST_FAIL, 100, str.c_str());
        if (ret != 0)
            pthis->tests_failed |= (1<<pthis->step);
        status += ret;
        pthis->step++;
    }
    pthis->Disconnect();
    pthis->running.store(false);
    return;
}

int LimeSDRTest::RunTests(bool nonblock)
{
    step = -1;
    if (running.load() == true)
        return -1;
    tp_start = std::chrono::steady_clock::now();
    running.store(true);
    std::string str = "[ TESTING STARTED ]\n->Start time: ";
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    str += std::ctime(&time);

    UpdateStatus(LMS_TEST_INFO, 0, str.c_str());

    if (Connect() != 0)
    {
        UpdateStatus(LMS_TEST_FAIL, 10, "Failed to connect");
        return -1;
    }

    step = 0;
    if (nonblock)
    {
        auto workThread = std::thread(_perform_tests, this);
        workThread.detach();
        return 0;
    }
    else
    {
        tests_failed  = 0;
        _perform_tests(this);
        return tests_failed;
    }
}

int LimeSDRTest::CheckDevice(std::string &str)
{
    if (running.load() == true)
        return 0;

    if (conn && conn->IsOpen())
        return 0;

    auto handles = LMS7_Device::GetDeviceList();

    if (handles.size() < 1)
    {
        str = "Error: No Board Detected";
        return -1;
    }
    else if (handles.size() > 1)
    {
        str = "Error: Multiple Board Detected";
        return -2;
    }
    str = "Device: ";

    str += handles[0].name;

    if (handles[0].serial.length() > 4)
    {
        str += " (";
        str += handles[0].serial + ")";
    }
    return 0;
}