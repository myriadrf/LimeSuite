/* 
 * File:   SteamerAPI.h
 * Author: ignas
 *
 * Created on May 5, 2016, 11:01 AM
 */

#ifndef STREAMERAPI_H
#define	STREAMERAPI_H

#include "LMS7002M.h"
#include "lime/LimeSuite.h"
#include <mutex>
#include "LMS64CProtocol.h" //TODO remove when reset usb is abstracted

#include <stdint.h>
#include <thread>
#include <atomic>
#include <vector>
#include <atomic>
#include <functional>
#include "dataTypes.h"
#include "fifo.h"



class StreamerAPI {
public:
    StreamerAPI(lime::IConnection* port);
    virtual ~StreamerAPI();
    
    virtual int SetupStream(lms_stream_conf_t conf);
    virtual int RecvStream(void **samples,size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms);
    virtual int SendStream(const void **samples,size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms);
    virtual int StartRx();
    virtual int StartTx();
    virtual int StopRx();
    virtual int StopTx();
    
protected:
    void ResetUSBFIFO();
    lms_stream_conf_t streamConf;
    lime::IConnection* streamPort;
    size_t channelsCount;
    bool rx_running;
    bool tx_running;
    int Start();
    int Stop();
    
private:
    
    int RecvStreamFloat(float **data, int16_t* buffer, size_t numSamples, uint64_t* ts, int &index, uint64_t* rx_meta, unsigned timeout_ms);
    int SendStreamFloat(const float **data, int16_t* buffer, size_t numSamples, uint64_t ts, int &index, uint64_t tx_meta, unsigned timeout_ms);
    int RecvStreamInt16(int16_t **data, int16_t* buffer, size_t numSamples, uint64_t* ts, int &index, uint64_t* rx_meta, unsigned timeout_ms);
    int SendStreamInt16(const int16_t **data, int16_t* buffer, size_t numSamples, uint64_t ts, int &index, uint64_t tx_meta, unsigned timeout_ms);
       
    int _read(int16_t *data, uint64_t *timestamp, uint64_t *metadata,unsigned timeout);
    int _write(int16_t *data, uint64_t timestamp, uint64_t metadata, unsigned timeout);
   

    std::mutex rx_lock;
    std::mutex tx_lock; 
    size_t packetsToBatch;
    int *rx_handles;
    char *rx_buffers;
    
    int *tx_handles;
    char *tx_buffers;
    bool *tx_bufferUsed;   
};

class StreamerFIFO : public StreamerAPI
{  
    public:
    StreamerFIFO(lime::IConnection* port);
    virtual ~StreamerFIFO(); 
    int SetupStream(lms_stream_conf_t conf);
    int RecvStream(void **samples,size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms);
    int SendStream(const void **samples,size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms);
    int StartRx();
    int StartTx();
    int StopRx();
    int StopTx();
        
    enum StreamDataFormat
    {
        STREAM_12_BIT_IN_16,
        STREAM_12_BIT_COMPRESSED,
    };

    enum STATUS
    {
        SUCCESS,
        FAILURE,
    };

    struct Stats
    {
        uint32_t rxBufSize;
        uint32_t rxBufFilled;
        uint32_t rxAproxSampleRate;
        uint32_t txBufSize;
        uint32_t txBufFilled;
        uint32_t txAproxSampleRate;
    };
    
    typedef std::function<void(const int status, const uint64_t &counter)> RxReportFunction;

/*!
 * Callback to retrieve a RxCommand structure.
 * @param [out] command the rx command
 * @return true if the command is valid
 */
typedef std::function<bool(lime::RxCommand &command)> RxPopCommandFunction;

    struct StreamerLTE_ThreadData
    {
        lime::IConnection* dataPort; //!< Connection interface
        lime::LMS_SamplesFIFO* FIFO; //!< FIFO (tx pops, rx pushes)
        std::atomic<bool>* terminate; //!< true exit loop
        std::atomic<uint32_t>* dataRate_Bps; //!< report rate
    };

private:

    lime::LMS_SamplesFIFO *mRxFIFO;
    lime::LMS_SamplesFIFO *mTxFIFO;

    static void ReceivePackets(const StreamerLTE_ThreadData &args);
    static void ReceivePacketsUncompressed(const StreamerLTE_ThreadData &args);
    static void TransmitPackets(const StreamerLTE_ThreadData &args);
    static void TransmitPacketsUncompressed(const StreamerLTE_ThreadData &args);

    std::atomic<bool> stopRx;
    std::atomic<bool> stopTx;

    std::thread threadRx;
    std::thread threadTx;

    std::atomic<uint32_t> mRxDataRate;
    std::atomic<uint32_t> mTxDataRate;

};
#endif	/* STEAMERAPI_H */

