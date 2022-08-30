#pragma once
#include <ConnectionRegistry.h>
#include "LMS64CProtocol.h"
#include <string>
#include <mutex>
#include <atomic>

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
protected:
    std::string mFilePath;
    int mFileDescriptor;
    bool isConnected;
};

}
