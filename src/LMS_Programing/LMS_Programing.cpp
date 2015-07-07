/**
@file LMS_Programing.cpp
@author Lime Microsystems
@brief Class for programming FPGA
*/
#include "LMS_Programing.h"
#include <fstream>
#include "lmsComms.h"
#include "lms7002_defines.h"
#include <sstream>
#include <string.h>
#include <iostream>
using namespace std;

LMS_Programing::LMS_Programing(LMScomms* pSerPort)
{
    mAbortPrograming.store(false);
    mUploadInProgress.store(false);
    m_serPort = pSerPort;
    m_data = NULL;
    m_data_size = 0;
    Info initInfo;
    initInfo.bytesSent = 0;
    initInfo.bytesCount= 0;
    initInfo.aborted = false;
    initInfo.deviceResponse = 0;
    mProgressInfo.store(initInfo);
}

LMS_Programing::~LMS_Programing()
{
    if (mUploadInProgress.load() == true)
        AbortPrograming();
    if(m_data)
        delete m_data;
}

/** @brief Loads program code from file
    @param filename file path
    @param type 0-binary, 1-hex
    @return 0:success, other: failure
*/
LMS_Programing::Status LMS_Programing::LoadFile(const char* filename, const int type)
{
    if (mUploadInProgress.load() == true)
        return UPLOAD_IN_PROGRESS;
    fstream fin;
    if(type == 0)
    {
        fin.open(filename, ios::in|ios::binary);
        if(!fin.good())
        {
            fin.close();
            return FILE_NOT_FOUND;
        }
        fin.seekg(0, ios_base::end);
        m_data_size = fin.tellg();
        if(m_data)
            delete[] m_data;
        m_data = new unsigned char[m_data_size];
        fin.seekg(0, ios_base::beg);
        fin.read((char*)m_data, m_data_size);
        fin.close();
        return SUCCESS;
    }
    return FAILURE;
}

/** @brief Loads program code from array
    @param array program code
    @param arraySize program code size in bytes
    @return 0:success, other:failure
*/
LMS_Programing::Status LMS_Programing::LoadArray(const unsigned char* array, const unsigned int arraySize)
{
    if (mUploadInProgress.load() == true)
        return UPLOAD_IN_PROGRESS;
    if(arraySize > 0)
    {
        m_data_size = arraySize;
        if(m_data)
            delete[] m_data;
        m_data = new unsigned char[m_data_size];
        memcpy(m_data, array, m_data_size);
        return SUCCESS;
    }
    else
        return FAILURE;
}

/** @brief Uploads program to controller
    @param device programmed device type 0:MyriadRF
    @param prog_mode programming mode
    @return 0:success, other: failure
*/
LMS_Programing::Status LMS_Programing::UploadProgram(const int device, const int prog_mode)
{
    if (mUploadInProgress.load() == true)
        return UPLOAD_IN_PROGRESS;
    mAbortPrograming.store(false);
    Info progress;
    progress.bytesSent = 0;
    progress.bytesCount= 0;
    progress.aborted = false;
    progress.deviceResponse = 0;
    mProgressInfo.store(progress);
    if(m_data_size == 0 && (device != 1) && (prog_mode != 2))
        return FAILURE;

    if(!m_serPort->IsOpen())
        return DEVICE_NOT_CONNECTED;
    mUploadInProgress.store(true);
    const int pktSize = 32;
    int data_left = m_data_size;
    unsigned char* data_src = m_data;    
    const int portionsCount = m_data_size/pktSize + (m_data_size%pktSize > 0) + 1; // +1 programming end packet
    int portionNumber;
    int status = 0;
    eCMD_LMS cmd;
    if(device == 0)
        cmd = CMD_MYRIAD_PROG;
    if(device == 1)
        cmd = CMD_ALTERA_FPGA_GW_WR;

    unsigned char ctrbuf[64];
    unsigned char inbuf[64];
    memset(ctrbuf, 0, 64);
    ctrbuf[0] = cmd;
    ctrbuf[1] = 0;
    ctrbuf[2] = 56;

    progress.bytesCount = m_data_size;
    for (portionNumber = 0; portionNumber<portionsCount && mAbortPrograming.load() != true; ++portionNumber)
    {
        int offset = 8;
        memset(&ctrbuf[offset], 0, 56);
        ctrbuf[offset+0] = prog_mode;
        ctrbuf[offset+1] = (portionNumber >> 24) & 0xFF;
        ctrbuf[offset+2] = (portionNumber >> 16) & 0xFF;
        ctrbuf[offset+3] = (portionNumber >> 8) & 0xFF;
        ctrbuf[offset+4] = portionNumber & 0xFF;
        unsigned char data_cnt = data_left > pktSize ? pktSize : data_left;
        ctrbuf[offset+5] = data_cnt;
        if(data_src != NULL)
        {
            memcpy(&ctrbuf[offset+24], data_src, data_cnt);
            data_src+=data_cnt;
        }

        m_serPort->Write(ctrbuf, 64);
        m_serPort->Read(inbuf, 64);

        data_left -= data_cnt;
        status = inbuf[1];
        progress.bytesSent += data_cnt;
        progress.deviceResponse = status;
        mProgressInfo.store(progress);
                
        if(status != STATUS_COMPLETED_CMD)
        {
#ifndef NDEBUG
            stringstream ss;
            ss << "Programming failed! Status: " << status2string(status) << endl;            
#endif
            progress.aborted = true;
            progress.deviceResponse = status;
            mProgressInfo.store(progress);
            return FAILURE;
        }        
        if (device == 1 && prog_mode == 2) //only one packet is needed to initiate bitstream from flash
            break;
#ifndef NDEBUG
        printf("programing: %6i/%i\r", portionNumber, portionsCount - 1);
#endif  
    }    
    mProgressInfo.store(progress);
    mUploadInProgress.store(false);
    if (mAbortPrograming.load() == true)
    {
        printf("\nProgramming aborted\n");
        progress.aborted = true;
        mProgressInfo.store(progress);
        return FAILURE;
    }    
#ifndef NDEBUG
	if ((device == 1 && prog_mode == 2) == false)
		printf("\nProgramming finished, %li bytes sent!\n", m_data_size);
	else
		printf("\nFPGA configuring initiated\n");
#endif    
    return SUCCESS;
}

/** @brief Starts programming procedure asynchronously
*/
LMS_Programing::Status LMS_Programing::StartUploadProgram(const int device, const int prog_mode)
{
    if (mUploadInProgress.load() == true)
        return UPLOAD_IN_PROGRESS;
    try
    {
        mProgramingThread = std::thread(&LMS_Programing::UploadProgram, this, device, prog_mode);
    }
    catch (...)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/** @returns programming progress
    Use this to poll progress if program uploading is done asynchronously
*/
LMS_Programing::Info LMS_Programing::GetProgressInfo() const
{
    return mProgressInfo.load();
}

/** @brief Aborts programming procedure
    Use this to stop programming if it is done asynchronously
*/
void LMS_Programing::AbortPrograming()
{
    if (mUploadInProgress.load() == true)
    {
        mAbortPrograming.store(true);
        mProgramingThread.join();
    }
}