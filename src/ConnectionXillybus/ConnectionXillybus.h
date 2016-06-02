/**
    @file ConnectionSTREAM.h
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <IConnection.h>
#include <LMS64CProtocol.h>
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <thread>

#ifndef __unix__
#include "windows.h"
#else
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

struct USBStreamService;

namespace lime{

class ConnectionXillybus : public LMS64CProtocol
{
public:
    ConnectionXillybus(const unsigned index);

    ~ConnectionXillybus(void);

	int Open(const unsigned index);
	void Close();
	bool IsOpen();
	int GetOpenedIndex();

	int Write(const unsigned char *buffer, int length, int timeout_ms = 0);
	int Read(unsigned char *buffer, int length, int timeout_ms = 0);

	virtual int BeginDataReading(char *buffer, long length);
	virtual int WaitForReading(int contextHandle, unsigned int timeout_ms);
	virtual int FinishDataReading(char *buffer, long &length, int contextHandle);
	virtual void AbortReading();
    virtual int ReadDataBlocking(char *buffer, long &length, int timeout_ms);

	virtual int BeginDataSending(const char *buffer, long length);
	virtual int WaitForSending(int contextHandle, unsigned int timeout_ms);
	virtual int FinishDataSending(const char *buffer, long &length, int contextHandle);
	virtual void AbortSending();

	uint64_t GetHardwareTimestamp(void);
	void SetHardwareTimestamp(const uint64_t now);
	double GetHardwareTimestampRate(void);

	//IConnection stream API implementation
	std::string SetupStream(size_t &streamID, const StreamConfig &config);
	void CloseStream(const size_t streamID);
	size_t GetStreamSize(const size_t streamID);
	bool ControlStream(const size_t streamID, const bool enable, const size_t burstSize = 0, const StreamMetadata &metadata = StreamMetadata());
	int ReadStream(const size_t streamID, void * const *buffs, const size_t length, const long timeout_ms, StreamMetadata &metadata);
	int WriteStream(const size_t streamID, const void * const *buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata);
	int ReadStreamStatus(const size_t streamID, const long timeout_ms, StreamMetadata &metadata);

	//hooks to update FPGA plls when baseband interface data rate is changed
	void UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate);
	void EnterSelfCalibration(const size_t channel);
	void ExitSelfCalibration(const size_t channel);
protected:
    int ConfigureFPGA_PLL(unsigned int pllIndex, const double interfaceClk_Hz, const double phaseShift_deg);
private:

    eConnectionType GetType(void)
    {
        return USB_PORT;
    }

    std::string m_hardwareName;
    int m_hardwareVer;

    bool isConnected;

    //! Stream service used by the stream and time API
    std::shared_ptr<USBStreamService> mStreamService;

#ifndef __unix__
    HANDLE hWrite;
    HANDLE hRead;	
    HANDLE hWriteStream;
    HANDLE hReadStream;	
    std::string writeStreamPort;
    std::string readStreamPort;
#else
    int hWrite;
    int hRead;	
    int hWriteStream;
    int hReadStream;
    std::string writeStreamPort;
    std::string readStreamPort;
#endif
};



class ConnectionXillybusEntry : public ConnectionRegistryEntry
{
public:
    ConnectionXillybusEntry(void);

    ~ConnectionXillybusEntry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);

private:
    #ifndef __unix__
    std::string DeviceName(unsigned int index);
    CCyUSBDevice *USBDevicePrimary;
    #else
    std::thread mUSBProcessingThread;
    void handle_libusb_events();
    std::atomic<bool> mProcessUSBEvents;
    #endif
};

}
