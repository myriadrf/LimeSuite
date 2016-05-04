/**
@file FPGA_common.h
@author Lime Microsystems
@brief Common functions used to work with FPGA
*/

#ifndef FPGA_COMMON_H
#define FPGA_COMMON_H
#include "IConnection.h"
#include <stdint.h>

namespace lime
{
namespace fpga
{

int InitializeStreaming(IConnection* serPort, const StreamConfig &config);
int StartStreaming(IConnection* serPort);
int StopStreaming(IConnection* serPort);
int ResetTimestamp(IConnection* serPort);

struct FPGA_PLL_clock
{
    double outFrequency;
    double phaseShift_deg;
    uint8_t index;
    bool bypass;
    double rd_actualFrequency;
};

int SetPllFrequency(IConnection* serPort, uint8_t pllIndex, const double inputFreq, FPGA_PLL_clock* outputs, const uint8_t clockCount);
int SetDirectClocking(IConnection* serPort, uint8_t clockIndex, const double inputFreq, const double phaseShift_deg);

}

}
#endif // FPGA_COMMON_H
