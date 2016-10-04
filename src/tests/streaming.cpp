#include "gtest/gtest.h"
#include "streamingFixture.h"
#include "IConnection.h"
#include "LMS7002M.h"
#include "math.h"
#include <thread>
#include <chrono>
#include "dataTypes.h"
using namespace std;
using namespace lime;
#define DRAW_GNU_PLOTS 0

#if DRAW_GNU_PLOTS
#include "gnuPlotPipe.h"
GNUPlotPipe gp;
#endif
struct complexFloat32_t
{
    float i;
    float q;
};

TEST_F (StreamingFixture, channelsRxAB)
{
    LMS7002M lmsControl;
    lmsControl.SetConnection(serPort, 0);
    lmsControl.ResetChip();
    //load initial settings to get samples
    lmsControl.UploadAll();
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    lmsControl.SetFrequencySX(LMS7002M::Tx, 1e6);
    lmsControl.SetFrequencySX(LMS7002M::Rx, 1e6);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    lmsControl.SetActiveChannel(LMS7002M::ChAB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);

    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1);
    lmsControl.SetActiveChannel(LMS7002M::ChB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0);
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    int ch = lmsControl.GetActiveChannelIndex();
    float cgenFreq = 30.72e6 * 4;
    lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
    auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
    auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
    serPort->UpdateExternalDataRate(0, txRate, rxRate);

    const int chCount = 2;

    //setup streaming
    size_t streamIds[chCount];
    StreamConfig config;
    config.isTx = false;
    config.format = StreamConfig::STREAM_12_BIT_COMPRESSED;
    //create streaming channel
    for(int i=0; i<chCount; ++i)
    {
        config.channelID = i;
        serPort->SetupStream(streamIds[i], config);
        ASSERT_NE(streamIds[i], size_t(~0));
    }

    const uint32_t streamsize = 680*32;
    complex16_t** buffers = new complex16_t*[chCount];
    for(int i=0; i<chCount; ++i)
        buffers[i] = new complex16_t[streamsize];

    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->ControlStream(streamIds[i], true);
        ASSERT_EQ(status, 0);
    }

    //Both channels are set to receive test signal
    //A channel full scale, B channel -6 dB
    //A channel amplitude should always be bigger than channel B
    //otherwise it would show channels data swapping
    lime::IStreamChannel::Metadata metadata;

#if DRAW_GNU_PLOTS
    gp.write("set title 'Channels Rx AB'\n");
    gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
    gp.write("plot '-' with points title 'A', '-' with points title 'B'\n");
#endif
    int samplesRead[chCount];
    for(int i=0; i<chCount; ++i)
    {
        samplesRead[i] = ((IStreamChannel*)streamIds[i])->Read((void *)buffers[i], streamsize, &metadata, 1000);
        EXPECT_EQ(streamsize, samplesRead[i]);
#if DRAW_GNU_PLOTS
        for(uint32_t j=0; j<streamsize; ++j)
            gp.writef("%i %i\n", buffers[i][j].i, buffers[i][j].q);
        gp.write("e\n");
        gp.flush();
#endif
    }

    for(int i=0; i < samplesRead[ch]; ++i)
    {
        //compare amplitudes from each channel, should be A > B
        float ampA = pow(buffers[0][i].i, 2) + pow(buffers[0][i].q, 2);
        float ampB = pow(buffers[1][i].i, 2) + pow(buffers[1][i].q, 2);
        //make sure both channels are receiving valid samples
        ASSERT_NE(0, ampA);
        ASSERT_NE(0, ampB);
        ASSERT_GT(ampA, ampB);
        constexpr int maxAmp = 2100;
        ASSERT_LT(ampA, maxAmp*maxAmp);
        ASSERT_LT(ampB, maxAmp*maxAmp);
    }

    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->ControlStream(streamIds[i], false);
        ASSERT_EQ(status, 0);
    }
    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->CloseStream(streamIds[i]);
        ASSERT_EQ(status, 0);
    }
    for(int i=0; i<chCount; ++i)
        delete []buffers[i];
    delete buffers;
}

TEST_F (StreamingFixture, channelsRxA)
{
    LMS7002M lmsControl;
    lmsControl.SetConnection(serPort, 0);
    lmsControl.ResetChip();
    //load initial settings to get samples
    lmsControl.UploadAll();
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    lmsControl.SetFrequencySX(LMS7002M::Tx, 1e6);
    lmsControl.SetFrequencySX(LMS7002M::Rx, 1e6);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    lmsControl.SetActiveChannel(LMS7002M::ChAB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);

    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1);
    lmsControl.SetActiveChannel(LMS7002M::ChB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0);
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    float cgenFreq = 30.72e6 * 4;
    lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
    auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
    auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
    serPort->UpdateExternalDataRate(0, txRate, rxRate);

    //setup streaming
    size_t streamId;
    StreamConfig config;
    config.channelID = 0;
    config.isTx = false;
    config.format = StreamConfig::STREAM_12_BIT_COMPRESSED;

    //create streaming channel
    serPort->SetupStream(streamId, config);
    ASSERT_NE(streamId, size_t(~0));

    const int streamsize = 680*32;
    complex16_t* buffer = new complex16_t[streamsize];

    auto status = serPort->ControlStream(streamId, true);

    //Both channels are set to receive test signal
    //A channel full scale, B channel -6 dB
    //A channel amplitude should always be bigger than channel B
    //otherwise it would show channels data swapping
    lime::IStreamChannel::Metadata metadata;
    int samplesRead = 0;
    samplesRead = ((IStreamChannel*)streamId)->Read((void *)buffer, streamsize, &metadata, 1000);
    EXPECT_EQ(streamsize, samplesRead);
#if DRAW_GNU_PLOTS
    gp.write("set title 'Channel Rx A'\n");
    gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
    gp.write("plot '-' with points title 'A'\n");
    for(int j=0; j<streamsize; ++j)
        gp.writef("%i %i\n", buffer[j].i, buffer[j].q);
    gp.write("e\n");
    gp.flush();
#endif
    for(int i=0; i < samplesRead; ++i)
    {
        float ampA = pow(buffer[i].i, 2) + pow(buffer[i].q, 2);
        //make sure both channels are receiving valid samples
        ASSERT_NE(0, ampA);
        const int minAmp = 1900*1900;
        const int maxAmp = 2100*2100;
        ASSERT_GT(ampA, minAmp);
        ASSERT_LT(ampA, maxAmp);
    }
    status = serPort->ControlStream(streamId, false);
    ASSERT_EQ(status, 0);
    status = serPort->CloseStream(streamId);
    ASSERT_EQ(status, 0);

    delete []buffer;
}

TEST_F (StreamingFixture, channelsRxB)
{
    LMS7002M lmsControl;
    lmsControl.SetConnection(serPort, 0);
    lmsControl.ResetChip();
    //load initial settings to get samples
    lmsControl.UploadAll();
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    lmsControl.SetFrequencySX(LMS7002M::Tx, 1e6);
    lmsControl.SetFrequencySX(LMS7002M::Rx, 1e6);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    lmsControl.SetActiveChannel(LMS7002M::ChAB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);

    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1);
    lmsControl.SetActiveChannel(LMS7002M::ChB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0);
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    float cgenFreq = 30.72e6 * 4;
    lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
    auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
    auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
    serPort->UpdateExternalDataRate(0, txRate, rxRate);

    //setup streaming
    size_t streamId;
    StreamConfig config;
    config.channelID = 1;
    config.isTx = false;
    config.format = StreamConfig::STREAM_12_BIT_COMPRESSED;

    //create streaming channel
    serPort->SetupStream(streamId, config);
    ASSERT_NE(streamId, size_t(~0));

    const int streamsize = 680*32;
    complex16_t* buffer = new complex16_t[streamsize];

    auto status = serPort->ControlStream(streamId, true);

    //Both channels are set to receive test signal
    //A channel full scale, B channel -6 dB
    //A channel amplitude should always be bigger than channel B
    //otherwise it would show channels data swapping
    lime::IStreamChannel::Metadata metadata;
    int samplesRead = 0;
    samplesRead = ((IStreamChannel*)streamId)->Read((void *)buffer, streamsize, &metadata, 1000);
    EXPECT_EQ(streamsize, samplesRead);
#if DRAW_GNU_PLOTS
    gp.write("set title 'Channel Rx B'\n");
    gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
    gp.write("plot '-' with points title 'B'\n");
    for(int j=0; j<streamsize; ++j)
        gp.writef("%i %i\n", buffer[j].i, buffer[j].q);
    gp.write("e\n");
    gp.flush();
#endif
    for(int i=0; i < samplesRead; ++i)
    {
        float ampB = pow(buffer[i].i, 2) + pow(buffer[i].q, 2);
        //make sure both channels are receiving valid samples
        ASSERT_NE(0, ampB);
        const int minAmp = 1900/2;
        const int maxAmp = 2100/2;
        ASSERT_GT(ampB, minAmp*minAmp);
        ASSERT_LT(ampB, maxAmp*maxAmp);
    }
    status = serPort->ControlStream(streamId, false);
    ASSERT_EQ(status, 0);
    status = serPort->CloseStream(streamId);
    ASSERT_EQ(status, 0);

    delete []buffer;
}

TEST_F (StreamingFixture, channelsRxAB_TxAB)
{
    LMS7002M lmsControl;
    lmsControl.SetConnection(serPort, 0);
    lmsControl.ResetChip();
    //load initial settings to get samples
    lmsControl.UploadAll();
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    lmsControl.SetFrequencySX(LMS7002M::Tx, 1e6);
    lmsControl.SetFrequencySX(LMS7002M::Rx, 1e6);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    lmsControl.SetActiveChannel(LMS7002M::ChAB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);

    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1);
    lmsControl.SetActiveChannel(LMS7002M::ChB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0);
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    int ch = lmsControl.GetActiveChannelIndex();
    float cgenFreq = 30.72e6 * 4;
    lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
    auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
    auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
    serPort->UpdateExternalDataRate(0, txRate, rxRate);

    const int chCount = 2;

    //setup streaming
    size_t streamIdsRx[chCount];
    size_t streamIdsTx[chCount];
    StreamConfig config;
    config.channelID = 0;
    config.isTx = false;
    config.format = StreamConfig::STREAM_12_BIT_COMPRESSED;

    //create streaming channel
    for(int i=0; i<chCount; ++i)
    {
        config.isTx = false;
        config.channelID = i;
        serPort->SetupStream(streamIdsRx[i], config);
        ASSERT_NE(streamIdsRx[i], size_t(~0));
    }

    //create streaming channel
    for(int i=0; i<chCount; ++i)
    {
        config.isTx = true;
        config.channelID = i;
        serPort->SetupStream(streamIdsTx[i], config);
        ASSERT_NE(streamIdsTx[i], size_t(~0));
    }

    const int streamsize = 680*32;
    complex16_t** buffers = new complex16_t*[chCount];
    for(int i=0; i<chCount; ++i)
        buffers[i] = new complex16_t[streamsize];

    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->ControlStream(streamIdsRx[i], true);
        ASSERT_EQ(status, 0);
    }
    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->ControlStream(streamIdsTx[i], true);
        ASSERT_EQ(status, 0);
    }

#if DRAW_GNU_PLOTS
    gp.write("set title 'Channels Rx AB 2 Tx AB'\n");
    gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
    gp.write("plot '-' with points");
    for(int i=1; i<chCount; ++i)
        gp.write(", '-' with points");
    gp.write("\n");
#endif
    //Both channels are set to receive test signal
    //A channel full scale, B channel -6 dB
    //A channel amplitude should always be bigger than channel B
    //otherwise it would show channels data swapping
    lime::IStreamChannel::Metadata metadata;
    int samplesRead[chCount];
    for(int i=0; i<chCount; ++i)
    {
        samplesRead[i] = ((IStreamChannel*)streamIdsRx[i])->Read((void *)buffers[i], streamsize, &metadata, 1000);
        EXPECT_EQ(streamsize, samplesRead[i]);
#if DRAW_GNU_PLOTS
        for(int j=0; j<streamsize; ++j)
            gp.writef("%i %i\n", buffers[i][j].i, buffers[i][j].q);
        gp.write("e\n");
        gp.flush();
#endif
    }

    for(int i=0; i < samplesRead[ch] && chCount > 1; ++i)
    {
        //compare amplitudes from each channel, should be A > B
        float ampA = pow(buffers[0][i].i, 2) + pow(buffers[0][i].q, 2);
        float ampB = pow(buffers[1][i].i, 2) + pow(buffers[1][i].q, 2);
        //make sure both channels are receiving valid samples
        ASSERT_NE(0, ampA);
        ASSERT_NE(0, ampB);
        ASSERT_GT(ampA, ampB);
        constexpr int maxAmp = 2100;
        ASSERT_LT(ampA, maxAmp*maxAmp);
        ASSERT_LT(ampB, maxAmp*maxAmp);
    }
    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->ControlStream(streamIdsRx[i], false);
        ASSERT_EQ(status, 0);
        status = serPort->ControlStream(streamIdsTx[i], false);
        ASSERT_EQ(status, 0);
    }
    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->CloseStream(streamIdsRx[i]);
        ASSERT_EQ(status, 0);
        status = serPort->CloseStream(streamIdsTx[i]);
        ASSERT_EQ(status, 0);
    }
    for(int i=0; i<chCount; ++i)
        delete []buffers[i];
    delete buffers;
}

TEST_F (StreamingFixture, DISABLED_Rx2TxLoopback)
{
    const auto testDuration = std::chrono::seconds(5);
    LMS7002M lmsControl;
    lmsControl.SetConnection(serPort, 0);
    lmsControl.ResetChip();
    //load initial settings to get samples
    lmsControl.UploadAll();
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    lmsControl.SetFrequencySX(LMS7002M::Tx, 1e6);
    lmsControl.SetFrequencySX(LMS7002M::Rx, 1e6);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    lmsControl.SetActiveChannel(LMS7002M::ChAB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);

    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1);
    lmsControl.SetActiveChannel(LMS7002M::ChB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0);
    lmsControl.SetActiveChannel(LMS7002M::ChA);

    float cgenFreq = 40e6;
    int statusInterface = lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
    ASSERT_EQ(0, statusInterface );
    auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
    auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
    printf("Rx rate: %g , Tx rate: %g\n", rxRate, txRate);
    serPort->UpdateExternalDataRate(0, txRate, rxRate);

    //setup streaming
    size_t streamIds[2];
    StreamConfig config;
    config.channelID = 0;
    config.isTx = false;
    config.format = StreamConfig::STREAM_12_BIT_COMPRESSED;
    serPort->SetupStream(streamIds[0], config);

    config.isTx = true;
    serPort->SetupStream(streamIds[1], config);

    const int streamsize = 16*1360;
    complex16_t buffers[streamsize];

    auto status = serPort->ControlStream(streamIds[0], true);
    ASSERT_EQ(status, 0);
    status = serPort->ControlStream(streamIds[1], true);
    ASSERT_EQ(status, 0);

    //this_thread::sleep_for(chrono::seconds(2));
    lime::IStreamChannel::Metadata metadata;
    auto testStart = chrono::high_resolution_clock::now();
    auto t2 = testStart;
    auto infoStart = testStart;
    auto infoPeriod = std::chrono::seconds(1);
    while(t2-testStart < testDuration)
    {
        int samplesRead = ((IStreamChannel*)streamIds[0])->Read((void *)buffers, streamsize, &metadata, 1000);
        ASSERT_EQ(samplesRead, streamsize);
        metadata.timestamp += 1024*1024;
        int samplesWrite = ((IStreamChannel*)streamIds[1])->Write((void *)buffers, samplesRead, &metadata, 1000);
        ASSERT_EQ(samplesWrite, samplesRead);
        t2 = chrono::high_resolution_clock::now();

        if (t2-infoStart >= infoPeriod)
        {
            infoStart = t2;
            IStreamChannel::Info rxStats = ((IStreamChannel*)streamIds[0])->GetInfo();
            IStreamChannel::Info txStats = ((IStreamChannel*)streamIds[1])->GetInfo();
            printf("Rx FIFO %2.1f%% \t\t Tx FIFO %2.1f%%\n", 100.0*rxStats.fifoItemsCount/rxStats.fifoSize, 100.0*txStats.fifoItemsCount/txStats.fifoSize);
        }
    }

    status = serPort->ControlStream(streamIds[0], false);
    ASSERT_EQ(status, 0);
    status = serPort->ControlStream(streamIds[1], false);
    ASSERT_EQ(status, 0);
    status = serPort->CloseStream(streamIds[0]);
    ASSERT_EQ(status, 0);
    status = serPort->CloseStream(streamIds[1]);
    ASSERT_EQ(status, 0);
}

TEST_F (StreamingFixture, channelsRxAB_Float)
{
    LMS7002M lmsControl;
    lmsControl.SetConnection(serPort, 0);
    lmsControl.ResetChip();
    //load initial settings to get samples
    lmsControl.UploadAll();
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(EN_ADCCLKH_CLKGN), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CLKH_OV_CLKL_CGEN), 2);
    lmsControl.SetFrequencySX(LMS7002M::Tx, 1e6);
    lmsControl.SetFrequencySX(LMS7002M::Rx, 1e6);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML1_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(LML2_MODE), 0);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(PD_RX_AFE2), 0);

    lmsControl.SetActiveChannel(LMS7002M::ChAB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(INSEL_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR1_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR2_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(GFIR3_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(AGC_BYP_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(CMIX_BYP_RXTSP), 1);

    lmsControl.SetActiveChannel(LMS7002M::ChA);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 1);
    lmsControl.SetActiveChannel(LMS7002M::ChB);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFCW_RXTSP), 1);
    lmsControl.Modify_SPI_Reg_bits(LMS7param(TSGFC_RXTSP), 0);
    lmsControl.SetActiveChannel(LMS7002M::ChA);
    int ch = lmsControl.GetActiveChannelIndex();
    float cgenFreq = 30.72e6 * 4;
    lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
    auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
    auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
    serPort->UpdateExternalDataRate(0, txRate, rxRate);

    const int chCount = 2;

    //setup streaming
    size_t streamIds[chCount];
    StreamConfig config;
    config.isTx = false;
    config.format = StreamConfig::STREAM_COMPLEX_FLOAT32;
    //create streaming channel
    for(int i=0; i<chCount; ++i)
    {
        config.channelID = i;
        serPort->SetupStream(streamIds[i], config);
        ASSERT_NE(streamIds[i], size_t(~0));
    }

    const uint32_t streamsize = 680*32;
    complexFloat32_t** buffers = new complexFloat32_t*[chCount];
    for(int i=0; i<chCount; ++i)
        buffers[i] = new complexFloat32_t[streamsize];

    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->ControlStream(streamIds[i], true);
        ASSERT_EQ(status, 0);
    }

    //Both channels are set to receive test signal
    //A channel full scale, B channel -6 dB
    //A channel amplitude should always be bigger than channel B
    //otherwise it would show channels data swapping
    lime::IStreamChannel::Metadata metadata;

#if DRAW_GNU_PLOTS
    gp.write("set title 'Channels Rx AB floats'\n");
    gp.write("set size square\n set xrange[-1:1]\n set yrange[-1:1]\n");
    gp.write("plot '-' with points title 'A', '-' with points title 'B'\n");
#endif
    int samplesRead[chCount];
    for(int i=0; i<chCount; ++i)
    {
        samplesRead[i] = ((IStreamChannel*)streamIds[i])->Read((void *)buffers[i], streamsize, &metadata, 1000);
        EXPECT_EQ(streamsize, samplesRead[i]);
#if DRAW_GNU_PLOTS
        for(uint32_t j=0; j<streamsize; ++j)
            gp.writef("%f %f\n", buffers[i][j].i, buffers[i][j].q);
        gp.write("e\n");
        gp.flush();
#endif
    }

    for(int i=0; i < samplesRead[ch]; ++i)
    {
        //compare amplitudes from each channel, should be A > B
        float ampA = pow(buffers[0][i].i, 2) + pow(buffers[0][i].q, 2);
        float ampB = pow(buffers[1][i].i, 2) + pow(buffers[1][i].q, 2);
        //make sure both channels are receiving valid samples
        ASSERT_NE(0, ampA);
        ASSERT_NE(0, ampB);
        ASSERT_GT(ampA, ampB);
        constexpr int maxAmp = 1;
        ASSERT_LT(ampA, maxAmp*maxAmp);
        ASSERT_LT(ampB, maxAmp*maxAmp);
    }

    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->ControlStream(streamIds[i], false);
        ASSERT_EQ(status, 0);
    }
    for(int i=0; i<chCount; ++i)
    {
        auto status = serPort->CloseStream(streamIds[i]);
        ASSERT_EQ(status, 0);
    }
    for(int i=0; i<chCount; ++i)
        delete []buffers[i];
    delete buffers;
}
