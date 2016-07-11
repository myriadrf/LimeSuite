#include "gtest/gtest.h"
#include "streamingFixture.h"
#include "IConnection.h"
#include "LMS7002M.h"
#include "math.h"
#include <thread>
#include <chrono>
using namespace std;
using namespace lime;

#include "gnuPlotPipe.h"

struct complex16_t
{
    int16_t i;
    int16_t q;
};

TEST_F (StreamingFixture, channelsRxAB)
{
    for(int repeatCount=0; repeatCount<1; ++repeatCount)
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
        auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
        auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
        int ch = lmsControl.GetActiveChannelIndex();
        float cgenFreq = 30.72e6 * 4;
        lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
        serPort->UpdateExternalDataRate(0, txRate, rxRate);

        const int chCount = 2;

        //setup streaming
        size_t streamIds[chCount];
        StreamConfig config;
        config.channelID = 0;
        config.isTx = false;
        config.format = StreamConfig::STREAM_12_BIT_IN_16;

        serPort->SetHardwareTimestamp(0);

        //create streaming channel
        for(int i=0; i<chCount; ++i)
        {
            config.channelID = i;
            serPort->SetupStream(streamIds[i], config);
            ASSERT_NE(streamIds[i], size_t(~0));
        }

        const uint32_t streamsize = 680;
        complex16_t** buffers = new complex16_t*[chCount];

        for(int i=0; i<chCount; ++i)
            buffers[i] = new complex16_t[streamsize];

        for(int i=0; i<chCount; ++i)
            auto ok = serPort->ControlStream(streamIds[i], true);

        uint16_t channelFlags = 0;
        serPort->ReadRegister(0x0007, channelFlags);
        ASSERT_EQ(0x3, channelFlags);

        //this_thread::sleep_for(chrono::seconds(2));

        //Both channels are set to receive test signal
        //A channel full scale, B channel -6 dB
        //A channel amplitude should always be bigger than channel B
        //otherwise it would show channels data swapping
        lime::IStreamChannel::Metadata metadata;
        GNUPlotPipe gp;
        for(int cnt=0; cnt<2; ++cnt)
        {
            int samplesRead[chCount];
            gp.write("set title 'Channels Rx AB'\n");
            gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
            gp.write("plot '-' with points");
            for(int i=1; i<chCount; ++i)
                gp.write(", '-' with points");
            gp.write("\n");
            for(int i=0; i<chCount; ++i)
            {
                samplesRead[i] = ((IStreamChannel*)streamIds[i])->Read((void *)buffers[i], streamsize, &metadata, 1000);
                EXPECT_EQ(streamsize, samplesRead[i]);
                for(uint32_t j=0; j<streamsize; ++j)
                    gp.writef("%i %i\n", buffers[i][j].i, buffers[i][j].q);
                gp.write("e\n");
                gp.flush();
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
            }
        }
        for(int i=0; i<chCount; ++i)
        {
            auto ok = serPort->ControlStream(streamIds[i], false);
            serPort->CloseStream(streamIds[i]);
        }
        delete buffers;
    }
}

TEST_F (StreamingFixture, channelsRxA)
{
    for(int repeatCount=0; repeatCount<1; ++repeatCount)
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
        auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
        auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
        int ch = lmsControl.GetActiveChannelIndex();
        float cgenFreq = 30.72e6 * 4;
        lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
        serPort->UpdateExternalDataRate(0, txRate, rxRate);

        const int chCount = 1;

        //setup streaming
        size_t streamIds[chCount];
        StreamConfig config;
        config.channelID = 0;
        config.isTx = false;
        config.format = StreamConfig::STREAM_12_BIT_IN_16;

        serPort->SetHardwareTimestamp(0);

        //create streaming channel
        for(int i=0; i<chCount; ++i)
        {
            config.channelID = i;
            serPort->SetupStream(streamIds[i], config);
            ASSERT_NE(streamIds[i], size_t(~0));
        }

        const int streamsize = 680;
        complex16_t** buffers = new complex16_t*[chCount];

        for(int i=0; i<chCount; ++i)
            buffers[i] = new complex16_t[streamsize];

        for(int i=0; i<chCount; ++i)
            auto ok = serPort->ControlStream(streamIds[i], true);

        uint16_t channelFlags = 0;
        serPort->ReadRegister(0x0007, channelFlags);
        ASSERT_EQ(0x1, channelFlags);

        //this_thread::sleep_for(chrono::seconds(2));

        //Both channels are set to receive test signal
        //A channel full scale, B channel -6 dB
        //A channel amplitude should always be bigger than channel B
        //otherwise it would show channels data swapping
        lime::IStreamChannel::Metadata metadata;
        GNUPlotPipe gp;
        for(int cnt=0; cnt<2; ++cnt)
        {
            int samplesRead[chCount];
            gp.write("set title 'Channel Rx A'\n");
            gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
            gp.write("plot '-' with points");
            for(int i=1; i<chCount; ++i)
                gp.write(", '-' with points");
            gp.write("\n");
            for(int i=0; i<chCount; ++i)
            {
                samplesRead[i] = ((IStreamChannel*)streamIds[i])->Read((void *)buffers[i], streamsize, &metadata, 1000);
                EXPECT_EQ(streamsize, samplesRead[i]);
                for(int j=0; j<streamsize; ++j)
                    gp.writef("%i %i\n", buffers[i][j].i, buffers[i][j].q);
                gp.write("e\n");
                gp.flush();
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
            }
        }
        for(int i=0; i<chCount; ++i)
        {
            auto ok = serPort->ControlStream(streamIds[i], false);
            serPort->CloseStream(streamIds[i]);
        }
        delete buffers;
    }
}

TEST_F (StreamingFixture, channelsRxB)
{
    for(int repeatCount=0; repeatCount<1; ++repeatCount)
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
        auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
        auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
        int ch = lmsControl.GetActiveChannelIndex();
        float cgenFreq = 30.72e6 * 4;
        lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
        serPort->UpdateExternalDataRate(0, txRate, rxRate);

        const int chCount = 1;

        //setup streaming
        size_t streamIds[chCount];
        StreamConfig config;
        config.channelID = 0;
        config.isTx = false;
        config.format = StreamConfig::STREAM_12_BIT_IN_16;

        serPort->SetHardwareTimestamp(0);

        //create streaming channel
        for(int i=0; i<chCount; ++i)
        {
            config.channelID = 1;
            serPort->SetupStream(streamIds[i], config);
            ASSERT_NE(streamIds[i], size_t(~0));
        }

        const int streamsize = 680;
        complex16_t** buffers = new complex16_t*[chCount];

        for(int i=0; i<chCount; ++i)
            buffers[i] = new complex16_t[streamsize];

        for(int i=0; i<chCount; ++i)
            auto ok = serPort->ControlStream(streamIds[i], true);

        uint16_t channelFlags = 0;
        serPort->ReadRegister(0x0007, channelFlags);
        ASSERT_EQ(0x2, channelFlags);

        //this_thread::sleep_for(chrono::seconds(2));

        //Both channels are set to receive test signal
        //A channel full scale, B channel -6 dB
        //A channel amplitude should always be bigger than channel B
        //otherwise it would show channels data swapping
        lime::IStreamChannel::Metadata metadata;
        GNUPlotPipe gp;
        for(int cnt=0; cnt<2; ++cnt)
        {
            int samplesRead[chCount];
            gp.write("set title 'Channel Rx B'\n");
            gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
            gp.write("plot '-' with points");
            for(int i=1; i<chCount; ++i)
                gp.write(", '-' with points");
            gp.write("\n");
            for(int i=0; i<chCount; ++i)
            {
                samplesRead[i] = ((IStreamChannel*)streamIds[i])->Read((void *)buffers[i], streamsize, &metadata, 1000);
                EXPECT_EQ(streamsize, samplesRead[i]);
                for(int j=0; j<streamsize; ++j)
                    gp.writef("%i %i\n", buffers[i][j].i, buffers[i][j].q);
                gp.write("e\n");
                gp.flush();
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
            }
        }
        for(int i=0; i<chCount; ++i)
        {
            auto ok = serPort->ControlStream(streamIds[i], false);
            serPort->CloseStream(streamIds[i]);
        }
        delete buffers;
    }
}

TEST_F (StreamingFixture, channelsRxAB_TxAB)
{
    for(int repeatCount=0; repeatCount<1; ++repeatCount)
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
        auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
        auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
        int ch = lmsControl.GetActiveChannelIndex();
        float cgenFreq = 30.72e6 * 4;
        lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
        serPort->UpdateExternalDataRate(0, txRate, rxRate);

        const int chCount = 2;

        //setup streaming
        size_t streamIdsRx[chCount];
        size_t streamIdsTx[chCount];
        StreamConfig config;
        config.channelID = 0;
        config.isTx = false;
        config.format = StreamConfig::STREAM_12_BIT_IN_16;

        serPort->SetHardwareTimestamp(0);

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

        const int streamsize = 680;
        complex16_t** buffers = new complex16_t*[chCount];

        for(int i=0; i<chCount; ++i)
            buffers[i] = new complex16_t[streamsize];

        for(int i=0; i<chCount; ++i)
            auto ok = serPort->ControlStream(streamIdsRx[i], true);
        for(int i=0; i<chCount; ++i)
            auto ok = serPort->ControlStream(streamIdsTx[i], true);

        uint16_t channelFlags = 0;
        serPort->ReadRegister(0x0007, channelFlags);
        ASSERT_EQ(0x3, channelFlags);

        //this_thread::sleep_for(chrono::seconds(2));

        //Both channels are set to receive test signal
        //A channel full scale, B channel -6 dB
        //A channel amplitude should always be bigger than channel B
        //otherwise it would show channels data swapping
        lime::IStreamChannel::Metadata metadata;
        GNUPlotPipe gp;
        for(int cnt=0; cnt<2; ++cnt)
        {
            int samplesRead[chCount];
            gp.write("set title 'Channels Rx AB Tx A'\n");
            gp.write("set size square\n set xrange[-2050:2050]\n set yrange[-2050:2050]\n");
            gp.write("plot '-' with points");
            for(int i=1; i<chCount; ++i)
                gp.write(", '-' with points");
            gp.write("\n");
            for(int i=0; i<chCount; ++i)
            {
                samplesRead[i] = ((IStreamChannel*)streamIdsRx[i])->Read((void *)buffers[i], streamsize, &metadata, 1000);
                EXPECT_EQ(streamsize, samplesRead[i]);
                for(int j=0; j<streamsize; ++j)
                    gp.writef("%i %i\n", buffers[i][j].i, buffers[i][j].q);
                gp.write("e\n");
                gp.flush();
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
            }
        }
        for(int i=0; i<chCount; ++i)
        {
            auto ok = serPort->ControlStream(streamIdsRx[i], false);
            ok = serPort->ControlStream(streamIdsTx[i], false);
            serPort->CloseStream(streamIdsRx[i]);
            serPort->CloseStream(streamIdsTx[i]);
        }
        delete buffers;
    }
}

TEST_F (StreamingFixture, DISABLED_perfTest)
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

    float cgenFreq = 100e6;
    int statusInterface = lmsControl.SetInterfaceFrequency(cgenFreq, 0, 0);
    auto txRate = lmsControl.GetSampleRate(LMS7002M::Tx, LMS7002M::ChA);
    auto rxRate = lmsControl.GetSampleRate(LMS7002M::Rx, LMS7002M::ChA);
    printf("Rx rate: %g , Tx rate: %g\n", rxRate, txRate);
    ASSERT_EQ(0, statusInterface );
    serPort->UpdateExternalDataRate(0, txRate, rxRate);
    int statusCGEN = lmsControl.SetFrequencyCGEN(cgenFreq);
    ASSERT_EQ(0, statusCGEN);

    //setup streaming
    size_t streamIds[2];
    StreamConfig config;
    config.channelID = 0;
    config.isTx = false;
    config.format = StreamConfig::STREAM_12_BIT_IN_16;
    serPort->SetupStream(streamIds[0], config);

    config.isTx = true;
    serPort->SetupStream(streamIds[1], config);

    const int streamsize = 16*1360;
    complex16_t buffers[streamsize];

    auto ok = serPort->ControlStream(streamIds[0], true);
    ok = serPort->ControlStream(streamIds[1], true);

    //this_thread::sleep_for(chrono::seconds(2));
    lime::IStreamChannel::Metadata metadata;
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();
    int infoTs = 1;
    while(t2-t1 < chrono::seconds(15))
    {
        uint64_t timestamp;
        int samplesRead = ((IStreamChannel*)streamIds[0])->Read((void *)buffers, streamsize, &metadata, 1000);
        metadata.timestamp += 1024*1024;
        int samplesWrite = ((IStreamChannel*)streamIds[1])->Write((void *)buffers, samplesRead, &metadata, 1000);
        t2 = chrono::high_resolution_clock::now();

        if (t2-t1 >= chrono::seconds(infoTs))
        {
            ++infoTs;
            IStreamChannel::Info rxStats = ((IStreamChannel*)streamIds[0])->GetInfo();
            IStreamChannel::Info txStats = ((IStreamChannel*)streamIds[1])->GetInfo();
            printf("Rx FIFO %2.1f%% \t\t Tx FIFO %2.1f%%\n", 100.0*rxStats.fifoItemsCount/rxStats.fifoSize, 100.0*txStats.fifoItemsCount/txStats.fifoSize);
        }
    }

    ok = serPort->ControlStream(streamIds[0], false);
    ok = serPort->ControlStream(streamIds[1], false);
    serPort->CloseStream(streamIds[0]);
    serPort->CloseStream(streamIds[1]);
}
