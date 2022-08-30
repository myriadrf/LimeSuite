#include "LitePCIe.h"
#include <unistd.h>
#include <fcntl.h>
#include "Logger.h"
#include <iostream>
#include <poll.h>

using namespace std;
using namespace lime;

LitePCIe::LitePCIe() : mFilePath(""), mFileDescriptor(-1), isConnected(false)
{
}

LitePCIe::~LitePCIe()
{
    Close();
}

int LitePCIe::Open(const char* deviceFilename, uint32_t flags)
{
    mFilePath = deviceFilename;
    mFileDescriptor = open(deviceFilename, flags); //O_RDWR
    if (mFileDescriptor < 0)
    {
        isConnected = false;
        lime::error("Failed to open Lite PCIe");
        return -1;
    }
    isConnected = true;
    return 0;
}

bool LitePCIe::IsOpen()
{
    return mFileDescriptor >= 0;
}

void LitePCIe::Close()
{
    if (mFileDescriptor >= 0)
        close(mFileDescriptor);
    mFileDescriptor = -1;
}

int LitePCIe::WriteControl(const uint8_t *buffer, const int length, int timeout_ms)
{
    return write(mFileDescriptor, buffer, length);
}

int LitePCIe::ReadControl(uint8_t *buffer, const int length, int timeout_ms)
{
    memset(buffer, 0, length);
    uint32_t status = 0;
    auto t1 = chrono::high_resolution_clock::now();
    do
    { //wait for status byte to change
        read(mFileDescriptor, &status, sizeof(status));
        if ((status & 0xFF00) != 0)
            break;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    } while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);

    if ((status & 0xFF00) == 0)
        throw std::runtime_error("LitePCIe read status timeout");
    return read(mFileDescriptor, buffer, length);
}

int LitePCIe::WriteRaw(const uint8_t *buffer, const int length, int timeout_ms)
{
    int bwritten = write(mFileDescriptor, buffer, length);
    // workaround, flush data
    write(mFileDescriptor, nullptr, 0);
    return bwritten;
}

int LitePCIe::ReadRaw(uint8_t *buffer, const int length, int timeout_ms)
{
    if (mFileDescriptor < 0)
        throw std::runtime_error("LitePCIe port not opened");

    int totalBytesReceived = 0;
    int bytesToRead = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
        const int toRead = length - totalBytesReceived;
        int bytesReceived = read(mFileDescriptor, buffer + totalBytesReceived, toRead);
        if (bytesReceived < 0)
            return bytesReceived;
        if (bytesReceived == 0)
        {
            pollfd desc;
            desc.fd = mFileDescriptor;
            desc.events = POLLIN;

            // poll seems useless here, it always instantly returns that data
            // is available, but following read still returns 0 bytes
            int ret = poll(&desc, 1, timeout_ms);
            if (ret < 0)
                return errno;
            else if (ret == 0) // timeout
                return totalBytesReceived;
            continue;
        }
        totalBytesReceived += bytesReceived;
        if (totalBytesReceived < length)
            bytesToRead -= bytesReceived;
        else
            break;
    } while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);

    return totalBytesReceived;
}

/*
// B.J.
int LitePCIe::ReadDPDBuffer(char *buffer, unsigned length)
{

    int totalBytesReaded = 0;
    uint16_t interface_cfg;

    ReadRegister(0x01A1, interface_cfg);  // CAP_RESETN
    interface_cfg = (interface_cfg & ~0x10); // reset bit 4
    WriteRegister(0x01A1, interface_cfg);

    ReadRegister(0x01A1, interface_cfg);
    interface_cfg = (interface_cfg | 0x10); // set bit 4
    WriteRegister(0x01A1, interface_cfg);

    StartReading(buffer, 2, 60);

    uint16_t len = length / 24;
    //std::cout << "[INFO] length: " << len << std::endl;
    WriteRegister(0x01A0, len); // lenght

    ReadRegister(0x01A1, interface_cfg);  // CAP_EN
    interface_cfg = (interface_cfg | 0x1); // set lsb
    WriteRegister(0x01A1, interface_cfg);

    for (int i = 0; i < 20; i++) // wait some time
        ReadRegister(0x01A1, interface_cfg);

    ReadRegister(0x01A1, interface_cfg);
    interface_cfg = (interface_cfg & ~0x1); // reset lsb
    WriteRegister(0x01A1, interface_cfg);

    totalBytesReaded = ReceiveData2(buffer, length, 2, 100);

    AbortReading(2);
    return totalBytesReaded;
}
// end B.J.
*/
