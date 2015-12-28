/**
@file LMS_Programing.h
@author Lime Microsystems
*/

#ifndef LMS_Programing_H
#define LMS_Programing_H

#include <atomic>
#include <thread>
class IConnection;

class LMS_Programing
{
public:
    enum Status
    {
        SUCCESS,
        FAILURE,
        FILE_NOT_FOUND,
        DEVICE_NOT_CONNECTED,
        UPLOAD_IN_PROGRESS
    };

    struct Info
    {
        int bytesSent;
        int bytesCount;
        bool aborted;
        unsigned char deviceResponse; //in case of failure device might give reason
    };

    LMS_Programing(IConnection* pSerPort);
    ~LMS_Programing();

    Status LoadFile(const char* filename, const int type);
    Status LoadArray(const unsigned char* array, const unsigned int arraySize);
    Status UploadProgram(const int device, const int prog_mode);

    //Creates thread that executes UploadProgram
    Status StartUploadProgram(const int device, const int prog_mode);
    Info GetProgressInfo() const;
    void AbortPrograming();
protected:
    std::thread mProgramingThread;

    std::atomic<int> bytesSent;
    std::atomic<int> bytesCount;
    std::atomic<bool> aborted;
    std::atomic<unsigned char> deviceResponse;

    std::atomic<bool> mAbortPrograming;
    std::atomic<bool> mUploadInProgress;
    IConnection* m_serPort;
    unsigned char* m_data;
    long m_data_size;
};

#endif // LMS_Programing_H

