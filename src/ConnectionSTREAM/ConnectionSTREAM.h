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
#include "CyAPI.h"
#else
#include <libusb-1.0/libusb.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#endif

struct USBStreamService;

namespace lime{

#define USB_MAX_CONTEXTS 64 //maximum number of contexts for asynchronous transfers

/** @brief Wrapper class for holding USB asynchronous transfers contexts
*/
class USBTransferContext
{
public:
	USBTransferContext() : used(false)
	{
		id = idCounter++;
		#ifndef __unix__
		inOvLap = new OVERLAPPED;
		memset(inOvLap, 0, sizeof(OVERLAPPED));
		inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
		context = NULL;
		#else
		transfer = libusb_alloc_transfer(0);
		bytesXfered = 0;
		bytesExpected = 0;
		done = 0;
		#endif
	}
	~USBTransferContext()
	{
		#ifndef __unix__
		CloseHandle(inOvLap->hEvent);
		delete inOvLap;
		#else
		libusb_free_transfer(transfer);
		#endif
	}
	bool reset()
	{
        if(used)
            return false;
        #ifndef __unix__
        CloseHandle(inOvLap->hEvent);
        memset(inOvLap, 0, sizeof(OVERLAPPED));
        inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
        #endif
        return true;
	}
	bool used;
    int id;
    static int idCounter;
	#ifndef __unix__
	PUCHAR context;
	OVERLAPPED *inOvLap;
	#else
	libusb_transfer* transfer;
	long bytesXfered;
	long bytesExpected;
	std::atomic<bool> done;
	std::mutex transferLock;
	std::condition_variable cv;
	#endif
};

class ConnectionSTREAM : public LMS64CProtocol
{
public:
    ConnectionSTREAM(void *ctx, const unsigned index, const int vid=-1, const int pid=-1);

    ~ConnectionSTREAM(void);

	int Open(const unsigned index, const int vid, const int pid);
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

	int ProgramFx3Ram(char *fileName);
	bool CheckUSB3();
protected:
    int ConfigureFPGA_PLL(unsigned int pllIndex, const double interfaceClk_Hz, const double phaseShift_deg);
private:

    eConnectionType GetType(void)
    {
        return USB_PORT;
    }

    std::string m_hardwareName;
    int m_hardwareVer;

	USBTransferContext contexts[USB_MAX_CONTEXTS];
	USBTransferContext contextsToSend[USB_MAX_CONTEXTS];

	bool isConnected;

#ifndef __unix__
	CCyFX3Device *USBDevicePrimary;
	//control endpoints for DigiRed
	CCyControlEndPoint *InCtrlEndPt3;
	CCyControlEndPoint *OutCtrlEndPt3;

    //control endpoints for DigiGreen
	CCyUSBEndPoint *OutCtrEndPt;
	CCyUSBEndPoint *InCtrEndPt;

    //end points for samples reading and writing
	CCyUSBEndPoint *InEndPt;
	CCyUSBEndPoint *OutEndPt;

#else
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    libusb_device_handle *dev_handle; //a device handle
    libusb_context *ctx; //a libusb session
    int read_firmware_image(const char *filename, unsigned char *buf);
    int fx3_usbboot_download(const char *filename);
    int ram_write(unsigned char *buf, unsigned int ramAddress, int len);
#endif

    //! Stream service used by the stream and time API
    std::shared_ptr<USBStreamService> mStreamService;

    std::mutex mExtraUsbMutex;
};



class ConnectionSTREAMEntry : public ConnectionRegistryEntry
{
public:
    ConnectionSTREAMEntry(void);

    ~ConnectionSTREAMEntry(void);

    std::vector<ConnectionHandle> enumerate(const ConnectionHandle &hint);

    IConnection *make(const ConnectionHandle &handle);

private:
    #ifndef __unix__
    std::string DeviceName(unsigned int index);
    CCyUSBDevice *USBDevicePrimary;
    #else
    libusb_context *ctx; //a libusb session
    std::thread mUSBProcessingThread;
    void handle_libusb_events();
    std::atomic<bool> mProcessUSBEvents;
    #endif
};

}
