#pragma once
#include <string>
#include <mutex>
#include <atomic>

#include "software/kernel/litepcie.h"
#include "software/kernel/config.h"

namespace lime{

class LitePCIe
{
public:
    LitePCIe();
    ~LitePCIe();

    int Open(const char* deviceFilename, uint32_t flags);
    void Close();
    bool IsOpen();

    // Write/Read for communicating to control end points (SPI, I2C...)
    int WriteControl(const uint8_t *buffer, int length, int timeout_ms = 100);
    int ReadControl(uint8_t *buffer, int length, int timeout_ms = 100);

    // Write/Read for samples streaming
    int WriteRaw(const uint8_t *buffer, int length, int timeout_ms = 100);
    int ReadRaw(uint8_t *buffer, int length, int timeout_ms = 100);

    const std::string& GetPathName() const { return mFilePath; };
    void SetPathName(const char* filePath) { mFilePath = std::string(filePath); };

    int GetFd() const { return mFileDescriptor; };

    void RxDMAEnable(bool enabled, uint32_t bufferSize, uint8_t irqPeriod);
    void TxDMAEnable(bool enabled);

    struct DMAInfo {
        uint8_t* rxMemory;
        uint8_t* txMemory;
        int bufferSize;
        int bufferCount;
    };
    DMAInfo GetDMAInfo() { return mDMA; };

    struct DMAState {
        uint32_t hwIndex;
        uint32_t swIndex;
        uint32_t bufferSize;
        bool enabled;
        bool genIRQ;
    };
    DMAState GetRxDMAState();
    DMAState GetTxDMAState();

    int SetRxDMAState(DMAState s);
    int SetTxDMAState(DMAState s);

    bool WaitRx();
    bool WaitTx();


protected:
    std::string mFilePath;
    int mFileDescriptor;
    bool isConnected;

    DMAInfo mDMA;
};

}