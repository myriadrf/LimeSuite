#ifndef STREAMER_NOVENA_H
#define STREAMER_NOVENA_H

#include "LMS_StreamBoard.h"

class LMScomms;

class StreamerNovena : public LMS_StreamBoard
{
public:
    StreamerNovena(LMScomms* dataPort);
    virtual ~StreamerNovena();

    virtual LMS_StreamBoard::Status StartReceiving(unsigned int fftSize);
    virtual LMS_StreamBoard::Status StopReceiving();

    virtual LMS_StreamBoard::Status StartCyclicTransmitting(const int16_t* isamples, const int16_t* qsamples, uint32_t framesCount);
    virtual LMS_StreamBoard::Status StopCyclicTransmitting();

    LMS_StreamBoard::Status SPI_write(uint16_t address, uint16_t data);
    uint16_t SPI_read(uint16_t address);
protected:
    static void ReceivePackets(StreamerNovena* pthis);
    static void TransmitPackets(StreamerNovena* pthis);
};
#endif

