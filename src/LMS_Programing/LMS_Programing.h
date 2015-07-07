/**
@file LMS_Programing.h
@author Lime Microsystems
*/

#ifndef LMS_Programing_H
#define LMS_Programing_H

#include <atomic>

class LMScomms;

class LMS_Programing
{
public:
    LMS_Programing(LMScomms* pSerPort);
    ~LMS_Programing();

    int LoadFile(const char* filename, const int type);
    int LoadArray(const unsigned char* array, const unsigned int arraySize);
    int UploadProgram(const int device, const int prog_mode);
    float GetProgress() const;
protected:
    std::atomic<float> mProgressPercent;
    LMScomms* m_serPort;
    unsigned char* m_data;
    long m_data_size;
};

#endif // LMS_Programing_H

