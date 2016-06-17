#include "gtest/gtest.h"
#include "streamingFixture.h"
#include "IConnection.h"
#include "LMS7002M.h"
#include "math.h"
#include <thread>
#include <chrono>
using namespace std;
using namespace lime;

TEST_F (StreamingFixture, startStop)
{
    for(int repeatCount=0; repeatCount<1; ++repeatCount)
    {
        LMS7002M lmsControl;
        lmsControl.SetConnection(serPort, 0);
        lmsControl.ResetChip();
        //load initial settings to get samples
        lmsControl.UploadAll();
        lmsControl.SetActiveChannel(LMS7002M::ChA);
        lmsControl.Modify_SPI_Reg_bits(EN_ADCCLKH_CLKGN, 0);
        lmsControl.Modify_SPI_Reg_bits(CLKH_OV_CLKL_CGEN, 2);
        lmsControl.SetFrequencySX(LMS7002M::Tx, 1e6);
        lmsControl.SetFrequencySX(LMS7002M::Rx, 1e6);
        lmsControl.Modify_SPI_Reg_bits(LML1_MODE, 0);
        lmsControl.Modify_SPI_Reg_bits(LML2_MODE, 0);
        lmsControl.Modify_SPI_Reg_bits(PD_RX_AFE2, 0);

        lmsControl.SetActiveChannel(LMS7002M::ChAB);
        lmsControl.Modify_SPI_Reg_bits(INSEL_RXTSP, 1);
        lmsControl.Modify_SPI_Reg_bits(GFIR1_BYP_RXTSP, 1);
        lmsControl.Modify_SPI_Reg_bits(GFIR2_BYP_RXTSP, 1);
        lmsControl.Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 1);
        lmsControl.Modify_SPI_Reg_bits(AGC_BYP_RXTSP, 1);

        lmsControl.SetActiveChannel(LMS7002M::ChA);
        lmsControl.Modify_SPI_Reg_bits(TSGFCW_RXTSP, 1);
        lmsControl.Modify_SPI_Reg_bits(TSGFC_RXTSP, 1);
        lmsControl.SetActiveChannel(LMS7002M::ChB);
        lmsControl.Modify_SPI_Reg_bits(TSGFCW_RXTSP, 1);
        lmsControl.Modify_SPI_Reg_bits(TSGFC_RXTSP, 0);
        lmsControl.SetActiveChannel(LMS7002M::ChA);

        auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::Channel::ChA);
        auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::Channel::ChA);
        int ch = lmsControl.GetActiveChannelIndex();
        float cgenFreq = 30.72e6 * 2;
        lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
        serPort->UpdateExternalDataRate(ch, txRate, rxRate);

        //setup streaming
        size_t streamId;
        StreamConfig config;
        config.channels.push_back(0);
        config.channels.push_back(1);
        config.isTx = false;
        config.format = StreamConfig::STREAM_12_BIT_IN_16;
        serPort->SetHardwareTimestamp(0);
        //create streaming channel
        serPort->SetupStream(streamId, config);
        ASSERT_NE(streamId, size_t(~0));

        StreamMetadata metadata;
        metadata.hasTimestamp = false;
        metadata.endOfBurst = false;
        auto ok = serPort->ControlStream(streamId, true, 680, metadata);

        const int chCount = 2;
        int16_t** buffers = new int16_t*[chCount];
        const int streamsize = serPort->GetStreamSize(streamId);
        for(int i=0; i<chCount; ++i)
            buffers[i] = new int16_t[streamsize*2];

        //Both channels are set to receive test signal
        //A channel full scale, B channel -6 dB
        //A channel amplitude should always be bigger than channel B
        //otherwise it would show channels data swapping
        for(int cnt=0; cnt<100; ++cnt)
        {
            int samplesRead = serPort->ReadStream(streamId, (void * const *)buffers, streamsize, 1000, metadata);
            ASSERT_EQ(streamsize, samplesRead);

            for(int i=0; i < samplesRead; i+=2)
            {
                //compare amplitudes from each channel, should be A > B
                float ampA = pow(buffers[0][i], 2) + pow(buffers[0][i+1], 2);
                float ampB = pow(buffers[1][i], 2) + pow(buffers[1][i+1], 2);
                //make sure both channels are receiving valid samples
                ASSERT_NE(0, ampA);
                ASSERT_NE(0, ampB);
                ASSERT_GT(ampA, ampB);
            }
        }
        ok = serPort->ControlStream(streamId, false, 680, metadata);
        serPort->CloseStream(streamId);
        for(int i=0; i<chCount; ++i)
            delete buffers[i];
        delete buffers;
    }
}
