#pragma once
#include <string>
#include <mutex>
#include <atomic>
#include <vector>

#include "software/kernel/litepcie.h"

namespace lime {

/** @brief Class for communicating with a PCIe device. */
class LitePCIe
{
  public:
    static std::vector<std::string> GetDevicesWithPattern(const std::string& regex);
    LitePCIe();
    ~LitePCIe();

    int Open(const std::string& deviceFilename, uint32_t flags);
    void Close();
    bool IsOpen();

    // Write/Read for communicating to control end points (SPI, I2C...)
    virtual int WriteControl(const uint8_t* buffer, int length, int timeout_ms = 100);
    virtual int ReadControl(uint8_t* buffer, int length, int timeout_ms = 100);

    // Write/Read for samples streaming
    int WriteRaw(const uint8_t* buffer, int length, int timeout_ms = 100);
    int ReadRaw(uint8_t* buffer, int length, int timeout_ms = 100);

    const std::string& GetPathName() const { return mFilePath; };
    void SetPathName(const std::string& filePath) { mFilePath = filePath; };

    int GetFd() const { return mFileDescriptor; };

    void RxDMAEnable(bool enabled, uint32_t bufferSize, uint8_t irqPeriod);
    void TxDMAEnable(bool enabled);

    /** @brief Structure for holding the Direct Memory Access (DMA) information. */
    struct DMAInfo {
        DMAInfo()
            : rxMemory(nullptr)
            , txMemory(nullptr)
            , bufferSize(0)
            , bufferCount(0)
        {
        }
        uint8_t* rxMemory;
        uint8_t* txMemory;
        int bufferSize;
        int bufferCount;
    };
    DMAInfo GetDMAInfo() { return mDMA; };

    /** @brief Structure for holding the current state of the Direct Memory Access (DMA). */
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

    void CacheFlush(bool isTx, bool toDevice, uint16_t index);

  protected:
    std::string mFilePath;
    DMAInfo mDMA;
    int mFileDescriptor;
    bool isConnected;
};

} // namespace lime
