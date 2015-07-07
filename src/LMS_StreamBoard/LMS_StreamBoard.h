#ifndef STREAM_BOARD_API_H
#define STREAM_BOARD_API_H

#include <stdint.h>

class LMScomms;

class LMS_StreamBoard
{
public:
    enum Status
    {
        STREAM_BRD_SUCCESS,
        STREAM_BRD_FAILURE
    };
    
    static Status CaptureIQSamples(LMScomms* serPort, int16_t *isamples, int16_t *qsamples, uint32_t framesCount, bool frameStart);
    static Status ConfigurePLL(LMScomms *serPort, const float fOutTx_MHz, const float fOutRx_MHz, const float phaseShift_deg);
};

#endif