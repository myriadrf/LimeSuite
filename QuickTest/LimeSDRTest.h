/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   LimeSDRTest.h
 * Author: ignas
 *
 * Created on August 25, 2016, 9:50 AM
 */

#ifndef LIMESDRTEST_H
#define LIMESDRTEST_H

#include "lms7_device.h"
#include <IConnection.h>
#include <atomic>

#define LMS_TEST_FAIL       -1
#define LMS_TEST_SUCCESS    0
#define LMS_TEST_INFO       1
#define LMS_TEST_LOGFILE    4

extern const std::vector<std::string>testNames;

class LimeSDRTest
{
    typedef std::function<int(int testID, int event, int progress, const char* msg)> TestCallback;
    
    struct RFTestData
    {
        float rxfreq;
        float txfreq;
        float peakval;
        float peakfreq;
        int padGain;
    };

public:
    LimeSDRTest(TestCallback cb);
    ~LimeSDRTest();
    int RunTests(bool nonblock = true);
    int CheckDevice(std::string &str);

private:
    int Connect();
    void Disconnect();
    int UpdateStatus(int event, int progress = -1, const char* msg = nullptr);

    int FPGA_EEPROM_Test();
    int InitFPGATest(unsigned test, double timeout);
    int GPIFClkTest();
    int VCTCXOTest();
    int ClockNetworkTest();

    int LMS7002mTest();

    int ConfigureLMS(const std::vector<uint32_t> &lmsConfigRF);
    int CollectAndCalculateFFT(float* results, unsigned fftSize);
    int Reg_write(uint16_t address, uint16_t data);
    uint16_t Reg_read(uint16_t address);
    bool RunTest(RFTestData &data);
    int RFTest();

    static std::string RFTestInfo(const RFTestData data, bool passed);	
    lime::LMS7_Device* device;
    lime::IConnection* conn;
    int step;
    TestCallback callback;
    static void _perform_tests(LimeSDRTest* pthis);
    int TransferLMS64C(unsigned char* packet);

    float nyquistFrequency;                 // sampling rate of LMS
    lime::LMS7002M *lmsControl;  // LMS7002M control
    std::atomic<bool> running;
    int tests_failed;
    std::chrono::steady_clock::time_point tp_start;
};



#endif /* LIMESDRTEST_H */

