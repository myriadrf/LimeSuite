/**
    @file ConnectionSTREAM.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#include "ConnectionSPI.h"
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <wiringPi.h>
#include <FPGA_common.h>
#include "Logger.h"

#include <fstream>
#include <thread>
#include <chrono>

#include <sys/resource.h>

using namespace std;

using namespace lime;

#define SPI_STREAM_SPEED_HZ 50000000


ConnectionSPI* ConnectionSPI::pthis = nullptr;
/** @brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionSPI::ConnectionSPI(const unsigned index) :
    fd_stream(-1),
    fd_stream_clocks(-1),
    fd_control_lms(-1),
    fd_control_fpga(-1),
    fd_control_dac(-1),
    int_pin(26)
{	
    pthis = this;
    if (Open(index) < 0)
        lime::error("Failed to open SPI device");
    
    std::ifstream file("/proc/device-tree/model");
    if (file.good())
    {
        std::string str;
        std::getline(file, str);
        lime::info(str.c_str());
        if (str.find("Module")!=std::string::npos)
            int_pin = 12;
    }
    
    wiringPiSetup();
    pinMode(int_pin, INPUT);
    pullUpDnControl(int_pin, PUD_OFF);
    wiringPiISR(int_pin, INT_EDGE_RISING, &ConnectionSPI::StreamISR);
    uint8_t id = 0;
    double val = 35487;
    CustomParameterWrite(&id, &val, 1, "");
}
/** @brief Closes connection to chip and deallocates used memory.
*/
ConnectionSPI::~ConnectionSPI()
{
    Close();
}

/** @brief Tries to open connected USB device and find communication endpoints.
    @return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int ConnectionSPI::Open(const unsigned index)
{
    Close();
    const uint32_t mode = 0;
    const uint8_t bits = 8;
    const uint32_t speed_stream = SPI_STREAM_SPEED_HZ;
    const uint32_t speed_control = 2000000;

    if ((fd_stream = open("/dev/spidev0.0", O_RDWR)) != -1)
    {
        if (ioctl(fd_stream, SPI_IOC_WR_MODE, &mode) < 0)
            lime::error("Failed to set SPI write mode");
        if (ioctl(fd_stream, SPI_IOC_RD_MODE, &mode) < 0)
            lime::error("Failed to set SPI read mode");
        if (ioctl(fd_stream, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (write)");
        if (ioctl(fd_stream, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (read)");
        if (ioctl(fd_stream, SPI_IOC_WR_MAX_SPEED_HZ, &speed_stream) < 0)
            lime::error("Failed to set SPI write max speed");
        if (ioctl(fd_stream, SPI_IOC_RD_MAX_SPEED_HZ, &speed_stream) < 0)
            lime::error("Failed to set SPI read max speed");
    }

    if ((fd_stream_clocks = open("/dev/spidev0.1", O_RDWR)) != -1)
    {
        if (ioctl(fd_stream_clocks, SPI_IOC_WR_MODE, &mode) < 0)
            lime::error("Failed to set SPI write mode");
        if (ioctl(fd_stream_clocks, SPI_IOC_RD_MODE, &mode) < 0)
            lime::error("Failed to set SPI read mode");
        if (ioctl(fd_stream_clocks, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (write)");
        if (ioctl(fd_stream_clocks, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (read)");
        if (ioctl(fd_stream_clocks, SPI_IOC_WR_MAX_SPEED_HZ, &speed_stream) < 0)
            lime::error("Failed to set SPI write max speed");
        if (ioctl(fd_stream_clocks, SPI_IOC_RD_MAX_SPEED_HZ, &speed_stream) < 0)
            lime::error("Failed to set SPI read max speed");
    }

    if ((fd_control_dac = open("/dev/spidev1.0", O_RDWR)) != -1)
    {
        if (ioctl(fd_control_dac, SPI_IOC_WR_MODE, &mode) < 0)
            lime::error("Failed to set SPI write mode");
        if (ioctl(fd_control_dac, SPI_IOC_RD_MODE, &mode) < 0)
            lime::error("Failed to set SPI read mode");
        if (ioctl(fd_control_dac, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (write)");
        if (ioctl(fd_control_dac, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (read)");
        if (ioctl(fd_control_dac, SPI_IOC_WR_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI write max speed");
        if (ioctl(fd_control_dac, SPI_IOC_RD_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI read max speed");
    }

    if ((fd_control_lms = open("/dev/spidev1.1", O_RDWR)) != -1)
    {
        if (ioctl(fd_control_lms, SPI_IOC_WR_MODE, &mode) < 0)
            lime::error("Failed to set SPI write mode");
        if (ioctl(fd_control_lms, SPI_IOC_RD_MODE, &mode) < 0)
            lime::error("Failed to set SPI read mode");
        if (ioctl(fd_control_lms, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (write)");
        if (ioctl(fd_control_lms, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (read)");
        if (ioctl(fd_control_lms, SPI_IOC_WR_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI write max speed");
        if (ioctl(fd_control_lms, SPI_IOC_RD_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI read max speed");
    }

    if ((fd_control_fpga = open("/dev/spidev1.2", O_RDWR)) != -1)
    {
        if (ioctl(fd_control_fpga, SPI_IOC_WR_MODE, &mode) < 0)
            lime::error("Failed to set SPI write mode");
        if (ioctl(fd_control_fpga, SPI_IOC_RD_MODE, &mode) < 0)
            lime::error("Failed to set SPI read mode");
        if (ioctl(fd_control_fpga, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (write)");
        if (ioctl(fd_control_fpga, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
            lime::error("Failed to set SPI bits per word (read)");
        if (ioctl(fd_control_fpga, SPI_IOC_WR_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI write max speed");
        if (ioctl(fd_control_fpga, SPI_IOC_RD_MAX_SPEED_HZ, &speed_control) < 0)
            lime::error("Failed to set SPI read max speed");
    }
    return IsOpen() ? 0 : -1;
}

/** @brief Closes communication to device.
*/
void ConnectionSPI::Close()
{
	if (fd_stream != -1)
		close(fd_stream);
	if (fd_stream_clocks != -1)
		close(fd_stream_clocks);
	if (fd_control_lms != -1)
		close(fd_control_lms);
	if (fd_control_fpga != -1)
		close(fd_control_fpga);
	if (fd_control_dac != -1)
		close(fd_control_dac);
}

/** @brief Returns connection status
    @return 1-connection open, 0-connection closed.
*/
bool ConnectionSPI::IsOpen()
{
    if (fd_stream < 0 || fd_stream_clocks < 0 || fd_control_lms < 0 || fd_control_fpga < 0)
        return false;
    return true;
}

int ConnectionSPI::TransferSPI(int fd, const void *tx, void *rx, uint32_t len)
{
    spi_ioc_transfer tr = { (unsigned long)tx,
                            (unsigned long)rx,
                            len,
                            2000000,
                            0,
                            8 };

    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
}

/***********************************************************************
 * LMS7002M SPI access
 **********************************************************************/
int ConnectionSPI::WriteLMS7002MSPI(const uint32_t *data, size_t size, unsigned periphID)
{
    uint32_t* readData = new uint32_t[size];
    uint8_t* writeData = new uint8_t[size*4];
    int ret = 0;

    for (unsigned i = 0; i < size; i++)
    {
    	writeData[4*i] = (data[i]>>24)&0xff;
    	writeData[4*i+1] = (data[i]>>16)&0xff;
    	writeData[4*i+2] = (data[i]>>8)&0xff;
    	writeData[4*i+3] = data[i]&0xff;
    }

    if (TransferSPI(fd_control_lms, writeData, readData, size*4)!=size*4)
    {
    	lime::error("Write LMS7: SPI transfer error");
    	ret = -1;
    }
    delete [] readData;
    delete [] writeData;
    return ret;
}

int ConnectionSPI::ReadLMS7002MSPI(const uint32_t *data_in, uint32_t *data_out, size_t size, unsigned periphID)
{
    uint8_t* readData = new uint8_t[size*4];
    uint8_t* writeData = new uint8_t[size*4];
    int ret = 0;

    for (unsigned i = 0; i < size; i++)
    {
    	writeData[4*i] = (data_in[i]>>24)&0xff;
    	writeData[4*i+1] = (data_in[i]>>16)&0xff;
    	writeData[4*i+2] = 0;
    	writeData[4*i+3] = 0;
    }

    if (TransferSPI(fd_control_lms, writeData, readData, size*4) != size*4)
    {
    	lime::error("Read LMS7: SPI transfer error");
    	ret = -1;
    }

    for (unsigned i = 0; i < size; i++)
    	data_out[i] = readData[4*i+3] | (readData[4*i+2]<<8);

    delete [] readData;
    delete [] writeData;
    return ret;
}

int ConnectionSPI::WriteRegisters(const uint32_t *addrs, const uint32_t *vals, const size_t size)
{
    uint32_t* readData = new uint32_t[size];
    uint8_t* writeData = new uint8_t[size*4];
    int ret = 0;
    for (unsigned i = 0; i < size; i++)
    {
    	writeData[4*i] = ((addrs[i]>>8)&0xff)|0x80;
    	writeData[4*i+1] = addrs[i]&0xff;
    	writeData[4*i+2] = (vals[i]>>8)&0xff;
    	writeData[4*i+3] = vals[i]&0xff;
    }

    if (TransferSPI(fd_control_fpga, writeData, readData, size*4) != size*4)
    {
    	lime::error("Write FPGA: SPI transfer error");
    	ret = -1;
    }

    delete [] readData;
    delete [] writeData;
    return ret;
}

int ConnectionSPI::ReadRegisters(const uint32_t *addrs, uint32_t *vals, const size_t size)
{
    uint8_t* readData = new uint8_t[size*4];
    uint8_t* writeData = new uint8_t[size*4];
    int ret = 0;
    for (unsigned i = 0; i < size; i++)
    {
    	writeData[4*i] = (addrs[i]>>8)&0xff;
    	writeData[4*i+1] = addrs[i]&0xff;
    	writeData[4*i+2] = 0;
    	writeData[4*i+3] = 0;
    }
    if (TransferSPI(fd_control_fpga, writeData, readData, size*4) != size*4)
    {
        lime::error("Read FPGA: SPI transfer error");
        ret = -1;
    }

    for (unsigned i = 0; i < size; i++)
    	vals[i] = readData[4*i+3] | (readData[4*i+2]<<8);

    delete [] readData;
    delete [] writeData;
    return ret;
}

int ConnectionSPI::CustomParameterRead(const uint8_t *ids, double *values, const size_t count, std::string* units)
{
    for (unsigned i = 0; i < count; i++)
        values[i] = ids[i] == 0 ? dac_value : 0;

    return 0;
}

int ConnectionSPI::CustomParameterWrite(const uint8_t *ids, const double *vals, const size_t count, const std::string& units)
{
   int ret = 0;

   for (unsigned i = 0; i < count; i++)
        if (ids[i] == 0)
        {
            uint16_t rx;
            dac_value = vals[i];
            uint16_t tx = (dac_value << 14);
            tx |= dac_value>>2;
            lime::info("DAC write: %d", dac_value);
            if (TransferSPI(fd_control_dac, &tx, &rx, 2) != 2)
            {
                lime::error("Read FPGA: SPI transfer error");
                ret = -1;
            }
        }

    return ret;
}

DeviceInfo ConnectionSPI::GetDeviceInfo(void)
{
    DeviceInfo info;
    info.deviceName = "LimeSDR-Mini";
    info.protocolVersion = "0";
    info.firmwareVersion = "0";
    info.expansionName = "EXP_BOARD_UNKNOWN";

    const uint32_t addrs[] = {0x0001, 0x0002, 0x0003};
    uint32_t vals[3] = {0};

    ReadRegisters(addrs, vals, 3);
    info.gatewareVersion = std::to_string(vals[0]);
    info.gatewareRevision = std::to_string(vals[1]);
    info.hardwareVersion = std::to_string(vals[2]);
    info.gatewareTargetBoard = "LimeSDR-Mini";
    return info;
}

int ConnectionSPI::DeviceReset(int ind)
{
    const uint32_t addrs[2] = {0x13, 0x13};
    uint32_t vals[2] = {};
    ReadRegisters(addrs, vals, 1);
    vals[0] = vals[0] & (~0x02);
    vals[1] = vals[0] | 0x02;
    WriteRegisters(addrs, vals, 2);
    return 0;
}

int ConnectionSPI::ResetStreamBuffers()
{  
    return 0;
}

int ConnectionSPI::GetBuffersCount() const
{
    return 1;
}

int ConnectionSPI::CheckStreamSize(int size) const
{
    return 1;
}

/**
    @brief Reads data from board
    @param buffer array where to store received data
    @param length number of bytes to read
    @param timeout read timeout in milliseconds
    @return number of bytes received
*/
int ConnectionSPI::ReceiveData(char *buffer, int length, int epIndex, int timeout_ms)
{
    const int count = length/sizeof(FPGA_DataPacket);
    int offset = 0;
    for (int i = 0; i <  count; i++)
    {
        if (WaitForReading(0, timeout_ms)== false)
            return i * sizeof(FPGA_DataPacket);
        FinishDataReading(buffer+offset, sizeof(FPGA_DataPacket), 0);
        offset += sizeof(FPGA_DataPacket);
    }
    return count * sizeof(FPGA_DataPacket);
}

void ConnectionSPI::AbortReading(int epIndex)
{
    return;
}

/**
    @brief  sends data to board
    @param *buffer buffer to send
    @param length number of bytes to send
    @param timeout data write timeout in milliseconds
    @return number of bytes sent
*/
int ConnectionSPI::SendData(const char *buffer, int length, int epIndex, int timeout_ms)
{
    const int count = length/sizeof(FPGA_DataPacket);
    int offset = 0;
    for (int i = 0; i <  count; i++)
    {
        BeginDataSending(buffer+offset, length, 0);
        if (WaitForSending(0, timeout_ms) == false)
            return i * sizeof(FPGA_DataPacket);
        offset += sizeof(FPGA_DataPacket);
    }
    return count * sizeof(FPGA_DataPacket);
}

void ConnectionSPI::AbortSending(int epIndex)
{
    return;
}

int ConnectionSPI::BeginDataReading(char* buffer, uint32_t length, int ep)
{
    return 0;
}

bool ConnectionSPI::WaitForReading(int contextHandle, unsigned int timeout_ms)
{
    auto t1 = chrono::high_resolution_clock::now();
    do
    {
        {
            std::lock_guard<std::mutex> lock(mRxStreamLock);
            if (!rxQueue.empty())
                return true;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
    while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);
    return false;
}

int ConnectionSPI::FinishDataReading(char* buffer, uint32_t length, int contextHandle)
{
    FPGA_DataPacket* packet = reinterpret_cast<FPGA_DataPacket*>(buffer);
    std::lock_guard<std::mutex> lock(mRxStreamLock);
    if (!rxQueue.empty())
    {
        *packet = rxQueue.front();
        rxQueue.pop();
        return sizeof(FPGA_DataPacket);;
    }
    return 0;
}

int ConnectionSPI::BeginDataSending(const char* buffer, uint32_t length, int ep)
{
    const FPGA_DataPacket* packet = reinterpret_cast<const FPGA_DataPacket*>(buffer);
    std::lock_guard<std::mutex> lock(mTxStreamLock);
    if (txQueue.size() < 10)
    {
        txQueue.push(*packet);
        return sizeof(FPGA_DataPacket);
    }
    return 0;
}

bool ConnectionSPI::WaitForSending(int contextHandle, uint32_t timeout_ms)
{
    auto t1 = chrono::high_resolution_clock::now();
    do
    {
        {
            std::lock_guard<std::mutex> lock(mTxStreamLock);
            if (txQueue.size() < 10)
                return true;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
    while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);
    return false;
}

int ConnectionSPI::FinishDataSending(const char* buffer, uint32_t length, int contextHandle)
{
    return contextHandle;
}

void ConnectionSPI::StreamISR()
{
    static const FPGA_DataPacket dummy_packet = {0};
    FPGA_DataPacket rx_packet;
    FPGA_DataPacket tx_packet;
    static uint64_t rx_timestamp = 1360;
    while (1)
    {
        pthis->mTxStreamLock.lock();
        if (pthis->txQueue.empty())
        {
            pthis->mTxStreamLock.unlock();
            tx_packet = dummy_packet;
        }
        else
        {
            tx_packet = pthis->txQueue.front();
            pthis->txQueue.pop();
            pthis->mTxStreamLock.unlock();
            if (tx_packet.counter < rx_timestamp+5*1360)
                continue;
        }
        break;
    }

    {
        spi_ioc_transfer tr = { (unsigned long)&tx_packet,
                                (unsigned long)&rx_packet,
                                sizeof(tx_packet),
                                SPI_STREAM_SPEED_HZ,
                                0,
                                8 };
        ioctl(pthis->fd_stream, SPI_IOC_MESSAGE(1), &tr);
    }
   
    rx_timestamp = rx_packet.counter;

    pthis->mRxStreamLock.lock();

    if (pthis->rxQueue.size() >= 10)
        pthis->rxQueue.pop();
    pthis->rxQueue.push(rx_packet);
    pthis->mRxStreamLock.unlock();
    
    {
        spi_ioc_transfer tr = { (unsigned long)&dummy_packet,
                                (unsigned long)&rx_packet,
                                2,
                                SPI_STREAM_SPEED_HZ,
                                0,
                                8 };
        ioctl(pthis->fd_stream_clocks, SPI_IOC_MESSAGE(1), &tr);
    }
}

int ConnectionSPI::ProgramWrite(const char *data, size_t length, int prog_mode, int device, ProgrammingCallback cb)
{
    if (device != 2)
    {
        lime::error("Unsupported programming target");
        return -1;
    }
    if (prog_mode != 1)
    {
        lime::error("Programming mode not supported");
        return -1;
    }
 
    wiringPiISR(int_pin, INT_EDGE_RISING, &ConnectionSPI::ProgramISR);
    uint32_t addr = 0x7F;
    uint32_t val = 0x00;
    WriteRegisters(&addr, &val, 1);
    val = 0x01;
    WriteRegisters(&addr, &val, 1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    program_ready.store(false);
    val = 0x00;
    WriteRegisters(&addr, &val, 1);
    val = 0x01;
    WriteRegisters(&addr, &val, 1);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    static unsigned char rx_buffer[4096];

    const int sizeUFM = 0x8000;
    const int sizeCFM0 = 0x42000;
    const int startUFM = 0x1000;
    const int startCFM0 = 0x4B000;

    if (length != startCFM0 + sizeCFM0)
    {
        lime::error("Invalid image file");
        return -1;
    }
    std::vector<char> buffer(sizeUFM + sizeCFM0);
    memcpy(buffer.data(), data + startUFM, sizeUFM);
    memcpy(buffer.data() + sizeUFM, data + startCFM0, sizeCFM0);

    unsigned data_sent = 0;
    while (data_sent < buffer.size())
    {
        int cnt = 0;
        auto t1 = chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < 3000)
            if (program_ready.load())
            {
                cnt = buffer.size() - data_sent;
                if (cnt > 4096)
                    cnt = 4096;
                {
                    spi_ioc_transfer tr = { (unsigned long)(&buffer[data_sent]),
                                            (unsigned long)&rx_buffer,
                                            cnt,
                                            SPI_STREAM_SPEED_HZ,
                                            0,
                                            8 };
                    ioctl(pthis->fd_stream, SPI_IOC_MESSAGE(1), &tr);
                }
                program_ready.store(false);
                {
                    spi_ioc_transfer tr = { (unsigned long)&rx_buffer,
                                            (unsigned long)&rx_buffer,
                                            2,
                                            SPI_STREAM_SPEED_HZ,
                                            0,
                                            8 };
                    ioctl(pthis->fd_stream_clocks, SPI_IOC_MESSAGE(1), &tr);
                }
                data_sent +=cnt;
                break;
            } 
        if (cnt == 0)
        {
            lime::error("timeout");
            break;
        }
        if(cb && cb(data_sent, buffer.size(), ""))
        {
            lime::info("aborted");
            break;
        }
    }
    
    wiringPiISR(int_pin, INT_EDGE_RISING, &ConnectionSPI::StreamISR);
    return 0;
}

void ConnectionSPI::ProgramISR()
{
    pthis->program_ready.store(true);
}


