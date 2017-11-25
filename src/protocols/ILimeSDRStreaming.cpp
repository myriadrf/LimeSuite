#include "ILimeSDRStreaming.h"
#include "ErrorReporting.h"
#include <assert.h>
#include "FPGA_common.h"
#include "Logger.h"

using namespace lime;

static const int MAX_CHANNEL_COUNT = 6;

ILimeSDRStreaming::ILimeSDRStreaming()
{
    for (int i = 0; i < MAX_CHANNEL_COUNT/2; i++)
    	mStreamers.push_back(new Streamer(this,i));
}
ILimeSDRStreaming::~ILimeSDRStreaming()
{
    for (unsigned i = 0; i < mStreamers.size() ; i++)
        delete mStreamers[i];
}

StreamChannel* ILimeSDRStreaming::SetupStream(const StreamConfig& config)
{
    if ( config.channelID >= MAX_CHANNEL_COUNT)
        return nullptr;
    unsigned index = config.channelID/2;
    return mStreamers[index]->SetupStream(config);
}

uint64_t ILimeSDRStreaming::GetHardwareTimestamp(void)
{
    return mStreamers[0]->GetHardwareTimestamp();
}

void ILimeSDRStreaming::SetHardwareTimestamp(const uint64_t now)
{
    mStreamers[0]->SetHardwareTimestamp(now);
}


