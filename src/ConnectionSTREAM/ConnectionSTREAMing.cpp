/**
    @file ConnectionSTREAMing.cpp
    @author Lime Microsystems
    @brief Implementation of STREAM board connection (streaming API)
*/

#include "ConnectionSTREAM.h"
#include "LMS_StreamBoard.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <complex>

#define STREAM_MTU 16384 //for now

std::string ConnectionSTREAM::SetupStream(size_t &streamID, const StreamConfig &config)
{
    streamID = ~0;

    if (config.isTx)
        return "ConnectionSTREAM::setupStream() no TX yet";

    if (config.format != StreamConfig::STREAM_COMPLEX_FLOAT32)
        return "ConnectionSTREAM::setupStream() only complex floats now";

    if (config.channels.size() != 2 or config.channels.at(0) != 0 or config.channels.at(1) != 1)
        return "SoapyIConnection::setupStream() only channels 0, 1 supported";

    streamID = size_t(new LMS_StreamBoard(this));
    return ""; //success
}

void ConnectionSTREAM::CloseStream(const size_t streamID)
{
    auto *stream = (LMS_StreamBoard *)streamID;
    delete stream;
}

size_t ConnectionSTREAM::GetStreamSize(const size_t streamID)
{
    return STREAM_MTU;
}

bool ConnectionSTREAM::ControlStream(const size_t streamID, const bool enable, const size_t burstSize, const StreamMetadata &metadata)
{
    auto *stream = (LMS_StreamBoard *)streamID;
    if (enable) stream->StartReceiving(STREAM_MTU);
    else stream->StopReceiving();
}

int ConnectionSTREAM::ReadStream(const size_t streamID, void * const *buffs, const size_t length, const long timeout_ms, StreamMetadata &metadata)
{
    auto *stream = (LMS_StreamBoard *)streamID;

    const auto exitTime(std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeout_ms));
    do
    {
        auto data = stream->GetIncomingData();
        if (data.samplesI.size() != 0)
        {
            //TODO
            // * both buffers
            // * remainder when numElems is smaller
            size_t N = std::min(data.samplesI.size(), length);
            auto ch0_data = (std::complex<float> *)buffs[0];
            for (size_t i = 0; i < N; i++)
            {
                ch0_data[i] = std::complex<float>(data.samplesI[i], data.samplesQ[i]);
            }
            return N;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    } while (std::chrono::high_resolution_clock::now() < exitTime);

    return 0;
}

int ConnectionSTREAM::WriteStream(const size_t streamID, const void * const *buffs, const size_t length, const long timeout_ms, const StreamMetadata &metadata)
{
    return -1;
}

void ConnectionSTREAM::UpdateExternalDataRate(const size_t channel, const double txRate, const double rxRate)
{
    //std::cout << "LMS_StreamBoard::ConfigurePLL(tx=" << txRate/1e6 << "MHz, rx=" << rxRate/1e6  << "MHz)" << std::endl;
    LMS_StreamBoard::ConfigurePLL(this, txRate/1e6, rxRate/1e6, 90);
}
