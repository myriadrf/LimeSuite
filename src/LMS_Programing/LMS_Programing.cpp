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
#include <chrono>
using namespace std;

LMS_Programing::LMS_Programing(LMScomms* pSerPort)
{
    m_serPort = pSerPort;
    m_data = NULL;
    m_data_size = 0;
    mProgressPercent = 0;
}

LMS_Programing::~LMS_Programing()
{
    if(m_data)
        delete m_data;
}

/** @brief Loads program code from file
    @param filename file path
    @param type 0-binary, 1-hex
    @return 0:success, -1:failure, -2:file not found
*/
int LMS_Programing::LoadFile(const char* filename, const int type)
{
    fstream fin;
    if(type == 0)
    {
        fin.open(filename, ios::in|ios::binary);
        if(!fin.good())
        {
            fin.close();
            return -2;
        }
        fin.seekg(0, ios_base::end);
        m_data_size = fin.tellg();
        if(m_data)
            delete[] m_data;
        m_data = new unsigned char[m_data_size];
        fin.seekg(0, ios_base::beg);
        fin.readsome((char*)m_data, m_data_size);
        fin.close();
        return 0;
    }
    return -1;
}

/** @brief Loads program code from array
    @param array program code
    @param arraySize program code size in bytes
    @return 0:success, -1:failure, -2:file not found
*/
int LMS_Programing::LoadArray(const unsigned char* array, const unsigned int arraySize)
{
    if(arraySize > 0)
    {
        m_data_size = arraySize;
        if(m_data)
            delete[] m_data;
        m_data = new unsigned char[m_data_size];
        memcpy(m_data, array, m_data_size);
        return 0;
    }
    else
        return -1;
}

/** @brief Uploads program to controller
    @param device programmed device type 0:MyriadRF
    @param prog_mode programming mode
    @return 0:success, -1:failure, -2:program not loaded; -3:board not connected
*/
int LMS_Programing::UploadProgram(const int device, const int prog_mode)
{
    mProgressPercent.store(0);
    if(m_data_size == 0 && (device != 1) && (prog_mode != 2))
        return -2;

    if(!m_serPort->IsOpen())
        return -3;

    int pktSize = 32;
    int data_left = m_data_size;
    unsigned char* data_src = m_data;
    // +1 programming end packet
    int portionsCount = m_data_size/pktSize + (m_data_size%pktSize > 0) + 1;
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

    long updateInterval_ms = 1000;
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    for(portionNumber=0; portionNumber<portionsCount; ++portionNumber)
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
        long len = 64;
        m_serPort->Read(inbuf, len);

        data_left -= data_cnt;

        mProgressPercent.store(100 * (float)portionNumber / (portionsCount - 1));
        status = inbuf[1];
        
        t2 = chrono::high_resolution_clock::now();
        if(status != STATUS_COMPLETED_CMD)
        {
#ifndef NDEBUG
            stringstream ss;
            ss << "Programming failed! Status: " << status2string(status) << endl;            
#endif
            mProgressPercent.store(100);
            return -1;
        }
        long timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >updateInterval_ms || portionNumber == portionsCount - 1)
        {
            
#ifndef NDEBUG
            cout << "Programming progress: " << mProgressPercent.load() << endl;
#endif
            t1 = t2;
        }        
        if(device == 1 && prog_mode == 2)
            break;
    }
    mProgressPercent.store(100);
#ifndef NDEBUG
    cout << "Programming progress: " << mProgressPercent.load() << endl;
	if ((device == 1 && prog_mode == 2) == false)
		printf("Programming finished, %li bytes sent!\n", m_data_size);
	else
		printf("FPGA configuring initiated\n");
#endif
    return 0;
}

float LMS_Programing::GetProgress() const
{
    return mProgressPercent.load();
}