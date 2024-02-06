#include "limesuite/SDRDevice.h"

using namespace lime;

SDRDevice::StreamConfig::Extras::Extras()
    : usePoll{ true }
    , negateQ{ false }
    , waitPPS{ false }
{
}

SDRDevice::StreamConfig::Extras::PacketTransmission::PacketTransmission()
    : samplesInPacket{ 0 }
    , packetsInBatch{ 0 }
{
}

SDRDevice::StreamConfig::StreamConfig()
    : format{ DataFormat::I16 }
    , linkFormat{ DataFormat::I16 }
    , bufferSize{ 0 }
    , hintSampleRate{ 0 }
    , alignPhase{ false }
    , statusCallback{ nullptr }
    , userData{ nullptr }
    , extraConfig{}
{
    channels[TRXDir::Rx] = {};
    channels[TRXDir::Tx] = {};
}

const char SDRDevice::Descriptor::DEVICE_NUMBER_SEPARATOR_SYMBOL = '@';
const char SDRDevice::Descriptor::PATH_SEPARATOR_SYMBOL = '/';
