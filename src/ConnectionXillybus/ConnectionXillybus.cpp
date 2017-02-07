/**
    @file ConnectionSTREAM.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection.
*/

#include "ConnectionXillybus.h"
#include "ErrorReporting.h"
#ifndef __unix__
#include "Windows.h"
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include "Si5351C.h"
#include <FPGA_common.h>
#include <LMS7002M.h>
#include <ciso646>

#include <thread>
#include <chrono>

using namespace std;

using namespace lime;

/** @brief Initializes port type and object necessary to communicate to usb device.
*/
ConnectionXillybus::ConnectionXillybus(const unsigned index)
{
    RxLoopFunction = bind(&ConnectionXillybus::ReceivePacketsLoop, this, std::placeholders::_1);
    TxLoopFunction = bind(&ConnectionXillybus::TransmitPacketsLoop, this, std::placeholders::_1);

    m_hardwareName = "";
    isConnected = false;
#ifndef __unix__
    hWrite = INVALID_HANDLE_VALUE;
    hRead = INVALID_HANDLE_VALUE;
    hWriteStream  = INVALID_HANDLE_VALUE;
    hReadStream  = INVALID_HANDLE_VALUE;
#else
    hWrite = -1;
    hRead = -1;
    hWriteStream = -1;
    hReadStream = -1;
#endif
    if (this->Open(index) != 0)
        std::cerr << GetLastErrorMessage() << std::endl;

    GetChipVersion();
    std::shared_ptr<Si5351C> si5351module(new Si5351C());
    si5351module->Initialize(this);
    si5351module->SetPLL(0, 25000000, 0);
    si5351module->SetPLL(1, 25000000, 0);
    si5351module->SetClock(0, 27000000, true, false);
    si5351module->SetClock(1, 27000000, true, false);
    for (int i = 2; i < 8; ++i)
        si5351module->SetClock(i, 27000000, false, false);
    Si5351C::Status status = si5351module->ConfigureClocks();
    if (status != Si5351C::SUCCESS)
    {
        std::cerr << "Warning: Failed to configure Si5351C" << std::endl;
        return;
    }
    status = si5351module->UploadConfiguration();
    if (status != Si5351C::SUCCESS)
        std::cerr << "Warning: Failed to upload Si5351C configuration" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); //some settle time
}

/** @brief Closes connection to chip and deallocates used memory.
*/
ConnectionXillybus::~ConnectionXillybus()
{
    Close();
}

/** @brief Tries to open connected USB device and find communication endpoints.
    @return Returns 0-Success, other-EndPoints not found or device didn't connect.
*/
int ConnectionXillybus::Open(const unsigned index)
{
    Close();
 #ifndef __unix__
    const char writePort[] = "\\\\.\\xillybus_write_8";
    const char readPort[] = "\\\\.\\xillybus_read_8";
#else
    const char writePort[] = "/dev/xillybus_write_8";
    const char readPort[] = "/dev/xillybus_read_8";
#endif

#ifndef __unix__
    hWrite = CreateFileA(writePort, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
    hRead = CreateFileA(readPort, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);

    // Check the results
    if (hWrite == INVALID_HANDLE_VALUE || hRead == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hWrite);
        CloseHandle(hRead);
        hWrite = INVALID_HANDLE_VALUE;
        hRead = INVALID_HANDLE_VALUE;
        ReportError("Failed to open Xillybus");
        return -1;
    }
#else
    hWrite = open(writePort, O_WRONLY | O_NOCTTY | O_NONBLOCK);
    hRead = open(readPort, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (hWrite == -1 || hRead == -1)
    {
        close(hWrite);
        close(hRead);
        hWrite = -1;
        hRead = -1;
        ReportError(errno);
        return -1;
    }
#endif
    return 0;
}

/** @brief Closes communication to device.
*/
void ConnectionXillybus::Close()
{
    isConnected = false;
#ifndef __unix__
    if (hWrite != INVALID_HANDLE_VALUE)
        CloseHandle(hWrite);
    hWrite = INVALID_HANDLE_VALUE;
    if (hRead != INVALID_HANDLE_VALUE)
        CloseHandle(hRead);
    hRead = INVALID_HANDLE_VALUE;

    if (hWriteStream != INVALID_HANDLE_VALUE)
        CloseHandle(hWriteStream);
    hWriteStream = INVALID_HANDLE_VALUE;
    if (hReadStream != INVALID_HANDLE_VALUE)
        CloseHandle(hReadStream);
    hReadStream = INVALID_HANDLE_VALUE;
#else
    if( hWrite >= 0)
        close(hWrite);
    hWrite = -1;
    if( hRead >= 0)
        close(hRead);
    hRead = -1;
    if( hWriteStream >= 0)
        close(hWriteStream);
    hWriteStream = -1;
    if( hReadStream >= 0)
        close(hReadStream);
    hReadStream = -1;
#endif
}

/** @brief Returns connection status
    @return 1-connection open, 0-connection closed.
*/
bool ConnectionXillybus::IsOpen()
{
#ifndef __unix__
    if (hWrite != INVALID_HANDLE_VALUE && hRead != INVALID_HANDLE_VALUE )
        return true;
#else
    if( hWrite != -1 && hRead != -1 )
        return true;
#endif
    return false;
}

/** @brief Sends given data buffer to chip through USB port.
    @param buffer data buffer, must not be longer than 64 bytes.
    @param length given buffer size.
    @param timeout_ms timeout limit for operation in milliseconds
    @return number of bytes sent.
*/
int ConnectionXillybus::Write(const unsigned char *buffer, const int length, int timeout_ms)
{
#ifndef __unix__
    if (hWrite == INVALID_HANDLE_VALUE)
#else
    if (hWrite == -1)
#endif
        return -1;

    int totalBytesWritten = 0;
    int bytesToWrite = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
#ifndef __unix__
        DWORD bytesSent = 0;
        OVERLAPPED	vOverlapped;
        memset(&vOverlapped, 0, sizeof(OVERLAPPED));
        vOverlapped.hEvent = CreateEvent(NULL, false, false, NULL);
        WriteFile(hWrite, buffer + totalBytesWritten, bytesToWrite, &bytesSent, &vOverlapped);
        if (::GetLastError() != ERROR_IO_PENDING)
        {
            CloseHandle(vOverlapped.hEvent);
            return totalBytesWritten;
        }
        std::this_thread::yield();
        DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, 500);
        if (dwRet == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(hWrite, &vOverlapped, &bytesSent, FALSE) == FALSE)
            {
                bytesSent = 0;
            }
        }
        else
        {
            CancelIo(hWrite);
            bytesSent = 0;
        }
        CloseHandle(vOverlapped.hEvent);
#else
        int bytesSent;
        if ((bytesSent  = write(hWrite, buffer+ totalBytesWritten, bytesToWrite))<0)
        {
            if(errno == EINTR || errno == EAGAIN)
                 continue;
            ReportError(errno);
            return totalBytesWritten;
        }
#endif
        totalBytesWritten += bytesSent;
        if (totalBytesWritten < length)
            bytesToWrite -= bytesSent;
        else
            break;

    }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);
#ifdef __unix__
    //Flush data to FPGA
    while (write(hWrite, NULL, 0) < 0)
    {
        if (errno == EINTR)
            continue;
        ReportError(errno);
        break;
    }
#endif
    return totalBytesWritten;
}

/** @brief Reads data coming from the chip through USB port.
    @param buffer pointer to array where received data will be copied, array must be
    big enough to fit received data.
    @param length number of bytes to read from chip.
    @param timeout_ms timeout limit for operation in milliseconds
    @return number of bytes received.
*/
int ConnectionXillybus::Read(unsigned char *buffer, const int length, int timeout_ms)
{
    memset(buffer, 0, length);
#ifndef __unix__
    if (hRead == INVALID_HANDLE_VALUE)
#else
    if (hRead == -1)
#endif
        return -1;

    int totalBytesReaded = 0;
    int bytesToRead = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
 #ifndef __unix__
        DWORD bytesReceived = 0;
        OVERLAPPED	vOverlapped;
        memset(&vOverlapped, 0, sizeof(OVERLAPPED));
        vOverlapped.hEvent = CreateEvent(NULL, false, false, NULL);
        ReadFile(hRead, buffer + totalBytesReaded, bytesToRead, &bytesReceived, &vOverlapped);
        if (::GetLastError() != ERROR_IO_PENDING)
        {
            CloseHandle(vOverlapped.hEvent);
            return totalBytesReaded;
        }
        std::this_thread::yield();
        DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, 1000);
        if (dwRet == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(hRead, &vOverlapped, &bytesReceived, TRUE) == FALSE)
            {
                bytesReceived = 0;
            }
        }
        else
        {
            CancelIo(hRead);
            bytesReceived = 0;
        }
        CloseHandle(vOverlapped.hEvent);
#else
        int bytesReceived;
        if ((bytesReceived = read(hRead, buffer+ totalBytesReaded, bytesToRead))<0)
        {
           if(errno == EINTR || errno == EAGAIN)
               continue;
           ReportError(errno);
           return totalBytesReaded;
        }
#endif
        totalBytesReaded += bytesReceived;
        if (totalBytesReaded < length)
           bytesToRead -= bytesReceived;
        else
           break;

    }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);

    return totalBytesReaded;
}

/**
    @brief Reads data from board
    @param buffer array where to store received data
    @param length number of bytes to read
    @param timeout read timeout in milliseconds
    @return number of bytes received
*/
int ConnectionXillybus::ReceiveData(char *buffer, const int length, const int timeout_ms)
{
#ifndef __unix__
    if (hReadStream == INVALID_HANDLE_VALUE)
    {
        hReadStream = CreateFileA("\\\\.\\xillybus_read_32", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
        if (hReadStream == INVALID_HANDLE_VALUE)
        {
            ReportError("Failed to open Xillybus");
            return -1;
        }
    }
#else
    if (hReadStream == -1)
    {
       if (( hReadStream = open("/dev/xillybus_read_32", O_RDONLY | O_NOCTTY | O_NONBLOCK))==-1)
       {
            ReportError(errno);
            return -1;
       }
    }
#endif

    int totalBytesReaded = 0;
    int bytesToRead = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
 #ifndef __unix__
        DWORD bytesReceived = 0;
        OVERLAPPED	vOverlapped;
        memset(&vOverlapped, 0, sizeof(OVERLAPPED));
        vOverlapped.hEvent = CreateEvent(NULL, false, false, NULL);
        ReadFile(hReadStream, buffer + totalBytesReaded, bytesToRead, &bytesReceived, &vOverlapped);
        if (::GetLastError() != ERROR_IO_PENDING)
        {
            CloseHandle(vOverlapped.hEvent);
            return totalBytesReaded;
        }
        DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, timeout_ms);
        if (dwRet == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(hReadStream, &vOverlapped, &bytesReceived, TRUE) == FALSE)
            {
                bytesReceived = 0;
            }
        }
        else
        {
            CancelIo(hReadStream);
            bytesReceived = 0;
        }
        CloseHandle(vOverlapped.hEvent);
#else
        int bytesReceived;
        if ((bytesReceived = read(hReadStream, buffer+ totalBytesReaded, bytesToRead))<0)
        {
            if(errno == EINTR || errno == EAGAIN)
                continue;
            ReportError(errno);
            return totalBytesReaded;
        }
#endif
        totalBytesReaded += bytesReceived;
        if (totalBytesReaded < length)
            bytesToRead -= bytesReceived;
        else
            break;

    }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);

    return totalBytesReaded;
}

/**
    @brief Aborts reading operations
*/
void ConnectionXillybus::AbortReading()
{
#ifndef __unix__
    if (hReadStream != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hReadStream);
	hReadStream = INVALID_HANDLE_VALUE;
    }
#else
    if (hReadStream >= 0)
    {
        close(hReadStream);
        hReadStream =-1;
    }
#endif
}

/**
    @brief  sends data to board
    @param *buffer buffer to send
    @param length number of bytes to send
    @param timeout data write timeout in milliseconds
    @return number of bytes sent
*/
int ConnectionXillybus::SendData(const char *buffer, const int length, const int timeout_ms)
{
#ifndef __unix__
    if (hWriteStream == INVALID_HANDLE_VALUE)
    {
        hWriteStream = CreateFileA("\\\\.\\xillybus_write_32", GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
        if (hWriteStream == INVALID_HANDLE_VALUE)
        {
            ReportError("Failed to open Xillybus");
            return -1;
        }
    }
#else
    if (hWriteStream == -1)
    {
       if ((hWriteStream = open("/dev/xillybus_write_32", O_WRONLY | O_NOCTTY | O_NONBLOCK))==-1)
       {
            ReportError(errno);
            return -1;
       }
    }
#endif
    int totalBytesWritten = 0;
    int bytesToWrite = length;
    auto t1 = chrono::high_resolution_clock::now();

    do
    {
#ifndef __unix__
        DWORD bytesSent = 0;
        OVERLAPPED	vOverlapped;
        memset(&vOverlapped, 0, sizeof(OVERLAPPED));
        vOverlapped.hEvent = CreateEvent(NULL, false, false, NULL);
        WriteFile(hWriteStream, buffer + totalBytesWritten, bytesToWrite, &bytesSent, &vOverlapped);
        if (::GetLastError() != ERROR_IO_PENDING)
        {
            CloseHandle(vOverlapped.hEvent);
            return totalBytesWritten;
        }
        DWORD dwRet = WaitForSingleObject(vOverlapped.hEvent, timeout_ms);
        if (dwRet == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(hWriteStream, &vOverlapped, &bytesSent, TRUE) == FALSE)
            {
                bytesSent = 0;
            }
        }
        else
        {
            CancelIo(hWriteStream);
            bytesSent = 0;
        }
        CloseHandle(vOverlapped.hEvent);
#else
        int bytesSent;
        if ((bytesSent  = write(hWriteStream, buffer+ totalBytesWritten, bytesToWrite))<0)
        {
            if(errno == EINTR || errno == EAGAIN)
                continue;
            ReportError(errno);
            return totalBytesWritten;
        }
#endif
        totalBytesWritten += bytesSent;
        if (totalBytesWritten < length)
            bytesToWrite -= bytesSent;
        else
            break;

    }while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < timeout_ms);
    //Flush data to FPGA
#ifdef __unix__
    while (write(hWriteStream, NULL, 0)<0)
    {
        if (errno == EINTR)
            continue;
        ReportError(errno);
        break;
    }
#endif
    return totalBytesWritten;
}

/**
	@brief Aborts sending operations
*/
void ConnectionXillybus::AbortSending()
{
#ifndef __unix__
    if (hWriteStream != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hWriteStream);
        hWriteStream = INVALID_HANDLE_VALUE;
    }
#else
    if (hWriteStream >= 0)
    {
        close(hWriteStream);
        hWriteStream = -1;
    }
#endif
}

/** @brief Configures Stream board FPGA clocks to Limelight interface
    @param tx Rx/Tx selection
    @param InterfaceClk_Hz Limelight interface frequency
    @param phaseShift_deg IQ phase shift in degrees
    @return 0-success, other-failure
*/
int ConnectionXillybus::ConfigureFPGA_PLL(unsigned int pllIndex, const double interfaceClk_Hz, const double phaseShift_deg)
{
    eLMS_DEV boardType = GetDeviceInfo().deviceName == GetDeviceName(LMS_DEV_LIMESDR_QPCIE) ? LMS_DEV_LIMESDR_QPCIE : LMS_DEV_UNKNOWN;
    const uint16_t busyAddr = 0x0021;
    if(IsOpen() == false)
        return ReportError(ENODEV, "ConnectionSTREAM: configure FPGA PLL, device not connected");

    uint16_t drct_clk_ctrl_0005 = 0;
    ReadRegister(0x0005, drct_clk_ctrl_0005);

    if(interfaceClk_Hz < 5e6)
    {
        //enable direct clocking
        WriteRegister(0x0005, drct_clk_ctrl_0005 | (1 << pllIndex));
        uint16_t drct_clk_ctrl_0006;
        ReadRegister(0x0006, drct_clk_ctrl_0006);
        drct_clk_ctrl_0006 = drct_clk_ctrl_0006 & ~0x3FF;
        const int cnt_ind = 1 << 5;
        const int clk_ind = pllIndex;
        drct_clk_ctrl_0006 = drct_clk_ctrl_0006 | cnt_ind | clk_ind;
        WriteRegister(0x0006, drct_clk_ctrl_0006);
        const uint16_t phase_reg_sel_addr = 0x0004;
        float inputClock_Hz = interfaceClk_Hz;
        const float oversampleClock_Hz = 100e6;
        //const int registerChainSize = 128;
        const float phaseShift_deg = 90;
        const float oversampleClock_ns = 1e9 / oversampleClock_Hz;
        const float phaseStep_deg = 360 * oversampleClock_ns*(1e-9) / (1 / inputClock_Hz);
        uint16_t phase_reg_select = (phaseShift_deg / phaseStep_deg)+0.5;
#ifndef NDEBUG
        const float actualPhaseShift_deg = 360 * inputClock_Hz / (1 / (phase_reg_select * oversampleClock_ns*1e-9));
        printf("reg value : %i\n", phase_reg_select);
        printf("input clock: %f\n", inputClock_Hz);
        printf("phase : %.2f/%.2f\n", phaseShift_deg, actualPhaseShift_deg);
#endif
        if(WriteRegister(phase_reg_sel_addr, phase_reg_select) != 0)
            return ReportError(EIO, "ConnectionSTREAM: configure FPGA PLL, failed to write registers");
        const uint16_t LOAD_PH_REG = 1 << 10;
        WriteRegister(0x0006, drct_clk_ctrl_0006 | LOAD_PH_REG);
        WriteRegister(0x0006, drct_clk_ctrl_0006);
        return 0;
    }

    //if interface frequency >= 5MHz, configure PLLs
    WriteRegister(0x0005, drct_clk_ctrl_0005 & ~(1 << pllIndex));

    //select FPGA index
    pllIndex = pllIndex & 0x1F;
    uint16_t reg23val = 0;
    if(ReadRegister(0x0003, reg23val) != 0)
        return ReportError(ENODEV, "ConnectionSTREAM: configure FPGA PLL, failed to read register");

    const uint16_t PLLCFG_START = 0x1;
    const uint16_t PHCFG_START = 0x2;
    const uint16_t PLLRST_START = 0x4;
    const uint16_t PHCFG_UPDN = 1 << 13;
    reg23val &= 0x1F << 3; //clear PLL index
    reg23val &= ~PLLCFG_START; //clear PLLCFG_START
    reg23val &= ~PHCFG_START; //clear PHCFG
    reg23val &= ~PLLRST_START; //clear PLL reset
    reg23val &= ~PHCFG_UPDN; //clear PHCFG_UpDn
    reg23val |= pllIndex << 3;

    uint16_t statusReg;
    bool done = false;
    uint8_t errorCode = 0;
    vector<uint32_t> addrs;
    vector<uint32_t> values;
    addrs.push_back(0x0023);
    values.push_back(reg23val); //PLL_IND
    addrs.push_back(0x0023);
    values.push_back(reg23val | PLLRST_START); //PLLRST_START
    WriteRegisters(addrs.data(), values.data(), values.size());

    if(boardType == LMS_DEV_LIMESDR_QPCIE) do //wait for reset to activate
    {
        ReadRegister(busyAddr, statusReg);
        done = statusReg & 0x1;
        errorCode = (statusReg >> 7) & 0xFF;
    } while(!done && errorCode == 0);
    if(errorCode != 0)
        return ReportError(EBUSY, "ConnectionSTREAM: error resetting PLL");

    addrs.clear();
    values.clear();
    addrs.push_back(0x0023);
    values.push_back(reg23val & ~PLLRST_START);

    //configure FPGA PLLs
    const float vcoLimits_MHz[2] = { 600, 1300 };
    int M, C;

    float fOut_MHz = interfaceClk_Hz / 1e6;
    float coef = 0.8*vcoLimits_MHz[1] / fOut_MHz;
    M = C = (int)coef;
    int chigh = (((int)coef) / 2) + ((int)(coef) % 2);
    int clow = ((int)coef) / 2;

    addrs.clear();
    values.clear();
    if(interfaceClk_Hz*M/1e6 > vcoLimits_MHz[0] && interfaceClk_Hz*M/1e6 < vcoLimits_MHz[1])
    {
        //bypass N
        addrs.push_back(0x0026);
        values.push_back(0x0001 | (M % 2 ? 0x8 : 0));

        addrs.push_back(0x0027);
        values.push_back(0x5550 | (C % 2 ? 0xA : 0)); //bypass c7-c1
        addrs.push_back(0x0028);
        values.push_back(0x5555); //bypass c15-c8

        addrs.push_back(0x002A);
        values.push_back(0x0101); //N_high_cnt, N_low_cnt
        addrs.push_back(0x002B);
        values.push_back(chigh << 8 | clow); //M_high_cnt, M_low_cnt

        for(int i = 0; i <= 1; ++i)
        {
            addrs.push_back(0x002E + i);
            values.push_back(chigh << 8 | clow); // ci_high_cnt, ci_low_cnt
        }

        float Fstep_us = 1 / (8 * fOut_MHz*C);
        float Fstep_deg = (360 * Fstep_us) / (1 / fOut_MHz);
        short nSteps = phaseShift_deg / Fstep_deg;

        addrs.push_back(0x0024);
        values.push_back(nSteps);

        addrs.push_back(0x0023);
        int cnt_ind = 0x3 & 0x1F;
        reg23val = reg23val | PHCFG_UPDN | (cnt_ind << 8);
        values.push_back(reg23val); //PHCFG_UpDn, CNT_IND

        addrs.push_back(0x0023);
        values.push_back(reg23val | PLLCFG_START); //PLLCFG_START
        if(WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
            ReportError(EIO, "ConnectionSTREAM: configure FPGA PLL, failed to write registers");
        if(boardType == LMS_DEV_LIMESDR_QPCIE) do //wait for config to activate
        {
            ReadRegister(busyAddr, statusReg);
            done = statusReg & 0x1;
            errorCode = (statusReg >> 7) & 0xFF;
        } while(!done && errorCode == 0);
        if(errorCode != 0)
            return ReportError(EBUSY, "ConnectionSTREAM: error configuring PLLCFG");

        addrs.clear();
        values.clear();
        addrs.push_back(0x0023);
        values.push_back(reg23val & ~PLLCFG_START); //PLLCFG_START
        addrs.push_back(0x0023);
        values.push_back(reg23val | PHCFG_START); //PHCFG_START
        if(WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
            ReportError(EIO, "ConnectionSTREAM: configure FPGA PLL, failed to write registers");
        if(boardType == LMS_DEV_LIMESDR_QPCIE) do
        {
            ReadRegister(busyAddr, statusReg);
            done = statusReg & 0x1;
            errorCode = (statusReg >> 7) & 0xFF;
        } while(!done && errorCode == 0);
        if(errorCode != 0)
            return ReportError(EBUSY, "ConnectionSTREAM: error configuring PHCFG");
        addrs.clear();
        values.clear();
        addrs.push_back(0x0023);
        values.push_back(reg23val & ~PHCFG_START); //PHCFG_START
        if(WriteRegisters(addrs.data(), values.data(), values.size()) != 0)
            ReportError(EIO, "ConnectionSTREAM: configure FPGA PLL, failed to write registers");
        return 0;
    }
    return ReportError(ERANGE, "ConnectionSTREAM: configure FPGA PLL, desired frequency out of range");
}
