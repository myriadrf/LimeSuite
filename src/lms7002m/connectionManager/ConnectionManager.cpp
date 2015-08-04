/**
    @file ConnectionManager.cpp
    @author Lime Microsystems (www.limemicro.com)
    @brief Implementation of various connection types to devices
*/

#include "ConnectionManager.h"
#include "ConnectionCOM.h"

#ifdef ENABLE_USB_CONNECTION
    #include "ConnectionUSB.h"
#endif
#ifdef ENABLE_SPI_CONNECTION
    #include "ConnectionSPI.h"
#endif

#include <sstream>
#include <iomanip>
#include <iostream>

/** @brief Creates connection interfaces
*/
ConnectionManager::ConnectionManager() : activeControlPort(NULL)
{
    mLogData = false;
    mOpenedDevice = -1;
    m_connections[IConnection::COM_PORT] = new ConnectionCOM();
#ifdef ENABLE_USB_CONNECTION
    m_connections[IConnection::USB_PORT] = new ConnectionUSB();
#endif
#ifdef ENABLE_SPI_CONNECTION
    m_connections[IConnection::SPI_PORT] = new ConnectionSPI();
#endif
}

/** @brief Destroys connection interfaces
*/
ConnectionManager::~ConnectionManager()
{
	for (auto iter = m_connections.begin(); iter != m_connections.end(); ++iter)
	{
		delete iter->second;
	}
}

/** @brief Checks if connection to device is opened
    @return True if device is connected
*/
bool ConnectionManager::IsOpen()
{
    return activeControlPort ? activeControlPort->IsOpen() : false;
}

/** @brief Opens connection to first available device
    @return True if connected to device
*/
bool ConnectionManager::Open()
{
    return Open(0);
}

/** @brief Connects to selected device
    @param i device index from device list
    @return 1:Success, 0:failure
*/
int ConnectionManager::Open(unsigned i)
{
    if(i >= mDevices.size())
        return 0;

    if(activeControlPort)
        activeControlPort->Close();
    switch(mDevices[i].port)
    {
    case IConnection::COM_PORT:
        activeControlPort = m_connections[IConnection::COM_PORT];
        break;
    case IConnection::USB_PORT:
		activeControlPort = m_connections[IConnection::USB_PORT];
        break;
    case IConnection::SPI_PORT:
		activeControlPort = m_connections[IConnection::SPI_PORT];
        break;
    default:
        return 0;
    }
    mOpenedDevice = -1;
    if( i < mDevices.size() )
    {
        if( activeControlPort->Open(mDevices[i].portIndex) )
        {
            mOpenedDevice = i;
            return 1;
        }
    }
    return 0;
}

/** @brief Closes connection to device
*/
void ConnectionManager::Close()
{
    if(activeControlPort)
    {
        activeControlPort->Close();
		//Notify(LMS_Message(MSG_BOARD_DISCONNECTED, "", 0, 0));
    }
    mOpenedDevice = -1;
}

/** @brief Finds all currently connected devices and forms device list
    @return number of devices found
*/
int ConnectionManager::RefreshDeviceList()
{
    mDeviceList.clear();
    mDevices.clear();
    DeviceInfo dev;
    for (auto iter = m_connections.begin(); iter != m_connections.end(); ++iter)
    {
        vector<string> names;
        IConnection *port = iter->second;
        if(port->RefreshDeviceList() > 0)
        {
            names = port->GetDeviceNames();
            for(unsigned i=0; i<names.size(); ++i)
            {
                dev.name = names[i];
                dev.port = iter->first;
                dev.portIndex = i;
                mDevices.push_back(dev);
            }
        }
    }
    for(unsigned i=0; i<mDevices.size(); ++i)
        mDeviceList.push_back(mDevices[i].name);
    return mDevices.size();
}

/** @brief Returns currently opened connection index
*/
int ConnectionManager::GetOpenedIndex()
{
    return mOpenedDevice;
}

/** @brief Writes given data to currently opened connection
    @param buffer outcomming data buffer
    @param length bytes to write
    @param timeout_ms timeout in milliseconds
    @return number of bytes written, on failure negative values
*/
int ConnectionManager::Write(const unsigned char *buffer, const int length, int timeout_ms)
{
    if(activeControlPort)
    {
        int bytesTransferred = activeControlPort->Write(buffer, length, timeout_ms);
#ifndef NDEBUG
        if(mLogData)
        {
            stringstream ss;
            ss << "WR(" <<  (bytesTransferred>=0?bytesTransferred: 0) << "): ";
            ss << std::hex << std::setfill('0');
            int repeatedZeros = 0;
            for(int i=length-1; i>=0; --i)
                if(buffer[i] == 0)
                    ++repeatedZeros;
                else break;
            if(repeatedZeros == 1)
                repeatedZeros = 0;
            repeatedZeros = repeatedZeros - (repeatedZeros & 0x1);
            for(int i=0; i<length-repeatedZeros; ++i)
                //casting to short to print as numbers
                ss << " " << std::setw(2) << (unsigned short)buffer[i];
            if(repeatedZeros > 1)
                ss << " (00 x " << std::dec << repeatedZeros << " times)";
            cout << ss.str() << endl;
        }
#endif
        return bytesTransferred;
    }
    return -1;
}

/** @brief Receives data from currently opened connection
    @param buffer incomming data buffer, must be big enough for length bytes
    @param length bytes to read
    @param timeout_ms timeout in milliseconds
    @return number of bytes received
*/
int ConnectionManager::Read(unsigned char *buffer, int length, int timeout_ms)
{
    if(activeControlPort)
    {
        int bytesTransferred = activeControlPort->Read(buffer, length, timeout_ms);
#ifndef NDEBUG
        if(mLogData)
        {
            stringstream ss;
            ss << "RD(" <<  (bytesTransferred>=0?bytesTransferred: 0) << "): ";
            ss << std::hex << std::setfill('0');
            int repeatedZeros = 0;
            for(int i=length-1; i>=0; --i)
                if(buffer[i] == 0)
                    ++repeatedZeros;
                else break;
            if(repeatedZeros == 2)
                repeatedZeros = 0;
            repeatedZeros = repeatedZeros - (repeatedZeros & 0x1);
            for(int i=0; i<length-repeatedZeros; ++i)
                //casting to short to print as numbers
                ss << " " << std::setw(2) << (unsigned short)buffer[i];
            if(repeatedZeros > 2)
                ss << " (00 x " << std::dec << repeatedZeros << " times)";
            cout << ss.str() << endl;
        }
#endif
        return bytesTransferred;
    }
    return -1;
}

int ConnectionManager::WriteStream(const char *buffer, int length)
{
	return 0;
}

int ConnectionManager::ReadStream(char *buffer, int length, unsigned int timeout_ms)
{
	/*int handle = activeControlPort->BeginDataReading(buffer, length);
    activeControlPort->WaitForReading(handle, timeout_ms);
	long received = length;
	activeControlPort->FinishDataReading(buffer, received, handle);
	return received;
    */
    long len = length;
    int status = activeControlPort->ReadDataBlocking(buffer, len, 0);
    return len;
}


int ConnectionManager::BeginDataReading(char *buffer, long length)
{
    return activeControlPort->BeginDataReading(buffer, length);
}
/**
@brief Blocks until data is received or set number of milliseconds have passed.
@param contextHandle handle returned by BeginDataReading()
@param timeout_ms number of milliseconds to wait
@return 1-data received, 0-data not received
*/
int ConnectionManager::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    return activeControlPort->WaitForReading(contextHandle, timeout_ms);
}
/**
@brief Finished asynchronous data reading.
@param buffer where to put received data
@param length number of bytes to read, will be changed to actual number of bytes received
@param contextHandle context handle returned by BeginDataReading()
@return received data length
*/
int ConnectionManager::FinishDataReading(char *buffer, long &length, int contextHandle)
{
    return activeControlPort->FinishDataReading(buffer, length, contextHandle);
}

/**
@brief Aborts reading operations
*/
void ConnectionManager::AbortReading()
{
    activeControlPort->AbortReading();
}

/**
@brief Start asynchronous data sending.
@param buffer data buffer to be sent
@param length number of bytes to send.
@return context handle
*/
int ConnectionManager::BeginDataSending(const char *buffer, long length)
{
    return activeControlPort->BeginDataSending(buffer, length);
}
/**
@brief Blocks until data is sent or set number of miliseconds have passed.
@param contextHandle handle returned by BeginDataReading()
@param timeout_ms number of miliseconds to wait
@return 1-data sent, 0-data not sent
*/
int ConnectionManager::WaitForSending(int contextHandle, unsigned int timeout_ms)
{
    return activeControlPort->WaitForSending(contextHandle, timeout_ms);
}
/**
@brief Finished asynchronous data sending.
@param buffer where to put received data
@param length number of bytes to send, will be changed to actual number of bytes sent
@param contextHandle context handle returned by BeginDataReading()
@return sent data length
*/
int ConnectionManager::FinishDataSending(const char *buffer, long &length, int contextHandle)
{
    return activeControlPort->FinishDataSending(buffer, length, contextHandle);
}

/**
@brief Aborts sending operations
*/
void ConnectionManager::AbortSending()
{
    activeControlPort->AbortSending();
}
