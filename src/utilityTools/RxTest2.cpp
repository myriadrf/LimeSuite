#include "LMS7002M.h"
#include "IConnection.h"
#include "ConnectionRegistry.h"
#include <iostream>
#include <iomanip>
#include "ErrorReporting.h"
#include "pllTest.h"
#include <getopt.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <chrono>
#include <assert.h>
#include <thread>
#include <chrono>
#include "fifo.h"
#include "FPGA_common.h"
#include "LMS64CProtocol.h"
//#define DRAW_GNUPLOT

#ifdef DRAW_GNUPLOT
#include "gnuPlotPipe.h"
#endif

using namespace std;
using namespace lime;

static void ResetUSBFIFO(LMS64CProtocol* port)
{
// TODO : USB FIFO reset command for IConnection
    if (port == nullptr) return;
    LMS64CProtocol::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_USB_FIFO_RST;
    ctrPkt.outBuffer.push_back(0x01);
    port->TransferPacket(ctrPkt);
    ctrPkt.outBuffer[0] = 0x00;
    port->TransferPacket(ctrPkt);
}

void PrintHelp()
{
    printf("--refClk= \tReference clock in Hz\n");
    printf("--config= \tChip configuration file to use for test\n");
    printf("--device= \tDevice index to automatically connect\n");
    printf("--cgen=   \tSet CGEN frequency in Hz\n");
    printf("--decimation= \tSet CGEN frequency in Hz\n");
    printf("--testCount= \tNumber of tests to run\n");
    printf("--readCount= \tHow many packets to read each test\n");
}

/** @brief Function dedicated for receiving data samples from board
    @param rxFIFO FIFO to store received data
    @param terminate periodically pooled flag to terminate thread
    @param dataRate_Bps (optional) if not NULL periodically returns data rate in bytes per second
*/
void ReceivePackets(const StreamerLTE_ThreadData &args)
{
    auto dataPort = args.dataPort;
    auto rxFIFO = args.FIFO;
    auto terminate = args.terminate;
    auto dataRate_Bps = args.dataRate_Bps;
    auto report = args.report;
    auto getCmd = args.getCmd;

    //at this point Rx must be enabled in FPGA
    unsigned long rxDroppedSamples = 0;
    const int channelsCount = rxFIFO->GetChannelsCount();
    uint32_t samplesCollected = 0;
    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = chrono::high_resolution_clock::now();

    const int bufferSize = 65536;
    const int buffersCount = 16; // must be power of 2
    const int buffersCountMask = buffersCount - 1;
    int handles[buffersCount];
    memset(handles, 0, sizeof(int)*buffersCount);
    vector<char>buffers;
    try
    {
        buffers.resize(buffersCount*bufferSize, 0);
    }
    catch (const std::bad_alloc &ex)
    {
        printf("Error allocating Rx buffers, not enough memory\n");
        return;
    }

    //temporary buffer to store samples for batch insertion to FIFO
    PacketFrame tempPacket;
    tempPacket.Initialize(channelsCount);

    for (int i = 0; i<buffersCount; ++i)
        handles[i] = dataPort->BeginDataReading(&buffers[i*bufferSize], bufferSize);

    int bi = 0;
    unsigned long totalBytesReceived = 0; //for data rate calculation
    int m_bufferFailures = 0;
    int16_t sample;

    uint32_t samplesReceived = 0;

    bool currentRxCmdValid = false;
    RxCommand currentRxCmd;
    size_t ignoreTxLateCount = 0;

    while (terminate->load() == false)
    {
        if (ignoreTxLateCount != 0) ignoreTxLateCount--;
        if (dataPort->WaitForReading(handles[bi], 1000) == false)
            ++m_bufferFailures;

        long bytesToRead = bufferSize;
        long bytesReceived = dataPort->FinishDataReading(&buffers[bi*bufferSize], bytesToRead, handles[bi]);
        if (bytesReceived > 0)
        {
            if (bytesReceived != bufferSize) //data should come in full sized packets
                ++m_bufferFailures;

            totalBytesReceived += bytesReceived;
            for (int pktIndex = 0; pktIndex < bytesReceived / sizeof(PacketLTE); ++pktIndex)
            {
                PacketLTE* pkt = (PacketLTE*)&buffers[bi*bufferSize];
                tempPacket.first = 0;
                tempPacket.timestamp = pkt[pktIndex].counter;

                int statusFlags = 0;
                uint8_t* pktStart = (uint8_t*)pkt[pktIndex].data;
                const int stepSize = channelsCount * 3;
                size_t numPktBytes = sizeof(pkt->data);

                auto byte0 = pkt[pktIndex].reserved[0];
                /*if ((byte0 & (1 << 3)) != 0 and ignoreTxLateCount == 0)
                {
                    uint16_t reg9;
                    dataPort->ReadRegister(0x0009, reg9);
                    Reg_write(dataPort, 0x0009, reg9 | (1 << 1));
                    Reg_write(dataPort, 0x0009, reg9 & ~(1 << 1));
                    if (report) report(STATUS_FLAG_TX_LATE, pkt[pktIndex].counter);
                    ignoreTxLateCount = 16;
                }*/

                if (report) report(STATUS_FLAG_TIME_UP, pkt[pktIndex].counter);

                if (getCmd)
                {
                    auto numPacketSamps = numPktBytes/stepSize;

                    //no valid command, try to pop a new command
                    if (not currentRxCmdValid) currentRxCmdValid = getCmd(currentRxCmd);

                    //command is valid, and this is an infinite read
                    //so we always replace the command if available
                    //the currentRxCmdValid variable remains true
                    else if (!currentRxCmd.finiteRead) getCmd(currentRxCmd);

                    //no valid command, just continue to the next pkt
                    if (not currentRxCmdValid) continue;

                    //handle timestamp logic
                    if (currentRxCmd.waitForTimestamp)
                    {
                        //the specified timestamp is late
                        //TODO report late condition to the rx fifo
                        //we are done with this current command
                        if (currentRxCmd.timestamp < tempPacket.timestamp)
                        {
                            std::cout << "L" << std::flush;
                            //until we can report late, treat it as asap:
                            currentRxCmd.waitForTimestamp = false;
                            //and put this one back in....
                            //currentRxCmdValid = false;
                            if (report) report(STATUS_FLAG_RX_LATE, currentRxCmd.timestamp);
                            continue;
                        }

                        //the specified timestamp is in a future packet
                        //continue onto the next packet
                        if (currentRxCmd.timestamp >= tempPacket.timestamp+numPacketSamps)
                        {
                            continue;
                        }

                        //the specified timestamp is within this packet
                        //adjust pktStart and numPktBytes for the offset
                        currentRxCmd.waitForTimestamp = false;
                        auto offsetSamps = currentRxCmd.timestamp-tempPacket.timestamp;
                        pktStart += offsetSamps*stepSize;
                        numPktBytes -= offsetSamps*stepSize;
                        numPacketSamps -= offsetSamps;
                        tempPacket.timestamp += offsetSamps;
                    }

                    //total adjustments for finite read
                    if (currentRxCmd.finiteRead)
                    {
                        //the current command has been depleted
                        //adjust numPktBytes to the requested end
                        if (currentRxCmd.numSamps <= numPacketSamps)
                        {
                            auto extraSamps = numPacketSamps-currentRxCmd.numSamps;
                            numPktBytes -= extraSamps*stepSize;
                            currentRxCmdValid = false;
                            statusFlags |= STATUS_FLAG_RX_END;
                        }
                        else currentRxCmd.numSamps -= numPacketSamps;
                    }
                }

                for (uint16_t b = 0; b < numPktBytes; b += stepSize)
                {
                    for (int ch = 0; ch < channelsCount; ++ch)
                    {
                        //I sample
                        sample = (pktStart[b + 1 + 3 * ch] & 0x0F) << 8;
                        sample |= (pktStart[b + 3 * ch] & 0xFF);
                        sample = sample << 4;
                        sample = sample >> 4;
                        tempPacket.samples[ch][samplesCollected].i = sample;

                        //Q sample
                        sample = pktStart[b + 2 + 3 * ch] << 4;
                        sample |= (pktStart[b + 1 + 3 * ch] >> 4) & 0x0F;
                        sample = sample << 4;
                        sample = sample >> 4;
                        tempPacket.samples[ch][samplesCollected].q = sample;
                    }
                    ++samplesCollected;
                    ++samplesReceived;
                }
                tempPacket.last = samplesCollected;

                uint32_t samplesPushed = rxFIFO->push_samples((const complex16_t**)tempPacket.samples, samplesCollected, channelsCount, tempPacket.timestamp, 10, statusFlags);
                if (samplesPushed != samplesCollected)
                {
                    rxDroppedSamples += samplesCollected - samplesPushed;
                    if (report) report(STATUS_FLAG_RX_DROP, pkt[pktIndex].counter);
                }
                samplesCollected = 0;
            }
        }
        else
        {
            ++m_bufferFailures;
        }

        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 100)
        {
            //total number of bytes sent per second
            double dataRate = 1000.0*totalBytesReceived / timePeriod;
            //total number of samples from all channels per second
            float samplingRate = 1000.0*samplesReceived / timePeriod;
            samplesReceived = 0;
            t1 = t2;
            totalBytesReceived = 0;
            if (dataRate_Bps)
                dataRate_Bps->store((long)dataRate);
            m_bufferFailures = 0;

#ifndef NDEBUG
            printf("Rx rate: %.3f MB/s Fs: %.3f MHz | dropped samples: %lu | TS: %li\n", dataRate / 1000000.0, samplingRate / 1000000.0, rxDroppedSamples, tempPacket.timestamp);
#endif
            rxDroppedSamples = 0;
        }

        // Re-submit this request to keep the queue full
        memset(&buffers[bi*bufferSize], 0, bufferSize);
        handles[bi] = dataPort->BeginDataReading(&buffers[bi*bufferSize], bufferSize);
        bi = (bi + 1) & buffersCountMask;
    }
    dataPort->AbortReading();
    for (int j = 0; j<buffersCount; j++)
    {
        long bytesToRead = bufferSize;
        dataPort->WaitForReading(handles[j], 1000);
        dataPort->FinishDataReading(&buffers[j*bufferSize], bytesToRead, handles[j]);
    }
}

int main(int argc, char** argv)
{
    double refClk = 30.72e6;
    double cgenFreq = 40e6;
    int decimation = 0;
    int deviceIndex = -1;
    unsigned int tryCount = 1;
    unsigned int readCount = 100;
    string outputFilename;
    string configFilename;

    while (1)
    {
        static struct option long_options[] =
        {
            {"refClk",      required_argument, 0, 'r'},
            {"config",      required_argument, 0, 'c'},
            {"device",      required_argument, 0, 'd'},
            {"help",        no_argument, 0, 'h'},
            {"cgen",        required_argument, 0, 'g'},
            {"decimation",  required_argument, 0, 'i'},
            {"testCount",    required_argument, 0, 't'},
            {"readCount",    required_argument, 0, 'u'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        int c = getopt_long (argc, argv, "r:c:d:g:i:t:h", long_options, &option_index);

        if (c == -1) //no parameters given
            break;
        switch (c)
        {
        case 'g':{
            stringstream ss;
            ss << optarg;
            ss >> cgenFreq;
            break;
        }
        case 'i':{
            stringstream ss;
            ss << optarg;
            ss >> decimation;
            break;
        }
        case 'r':{
            stringstream ss;
            ss << optarg;
            ss >> refClk;
            break;
        }
        case 'o':
            outputFilename = string(optarg);
            break;
        case 'c':{
            stringstream ss;
            ss << optarg;
            ss >> configFilename;
            break;
        }
        case 'd':{
            stringstream ss;
            ss << optarg;
            ss >> deviceIndex;
            break;
        }
        case 't':{
            stringstream ss;
            ss << optarg;
            ss >> tryCount;
            break;
        }
        case 'u':{
            stringstream ss;
            ss << optarg;
            ss >> readCount;
            break;
        }
        case 'h':
            PrintHelp();
            return 0;
            break;
        case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            abort();
        }
    }

    IConnection* serPort;
    std::vector<lime::ConnectionHandle> cachedHandles = ConnectionRegistry::findConnections();
    if(cachedHandles.size() == 0)
    {
        cout << "No devices found" << endl;
        return -1;
    }
    if(cachedHandles.size() == 1) //open the only available device
        serPort = ConnectionRegistry::makeConnection(cachedHandles.at(0));
    else //display device selection
    {
        if(deviceIndex < 0)
        {
            cout << "Device list:" << endl;
            for (size_t i = 0; i < cachedHandles.size(); i++)
               cout << setw(2) << i << ". " << cachedHandles[i].name << endl;
            cout << "Select device index (0-" << cachedHandles.size()-1 << "): ";
            int selection = 0; cin >> selection;
            selection = selection % cachedHandles.size();
            serPort = ConnectionRegistry::makeConnection(cachedHandles.at(selection));
        }
        else
            serPort = ConnectionRegistry::makeConnection(cachedHandles.at(deviceIndex));
    }
    if(serPort == nullptr)
    {
        cout << "Failed to connected to device" << endl;
        return -1;
    }
    DeviceInfo info = serPort->GetDeviceInfo();
    cout << "\nConnected to: " << info.deviceName
    << " FW: " << info.firmwareVersion << " HW: " << info.hardwareVersion << endl;

    int successCount = 0;
    int swapCount = 0;
    int invalidSamplesCount = 0;
    int failuresToGetData = 0;
    for(int c = 0; c<tryCount; ++c)
    {
        LMS7002M* lmsControl = new LMS7002M();
        lmsControl->SetConnection(serPort, 0);
        lmsControl->ResetChip();
        serPort->SetReferenceClockRate(refClk);
        if(configFilename.length() > 0)
        {
            if(lmsControl->LoadConfig(configFilename.c_str()) != 0)
            {
                cout << GetLastErrorMessage() << endl;
                return -1;
            }
        }
        else
        {
            lmsControl->UploadAll(); //upload software defaults to chip
            lmsControl->SetActiveChannel(LMS7002M::ChA);
            lmsControl->Modify_SPI_Reg_bits(EN_ADCCLKH_CLKGN, 0);
            lmsControl->Modify_SPI_Reg_bits(CLKH_OV_CLKL_CGEN, 2);
            lmsControl->SetFrequencySX(LMS7002M::Tx, 1e6);
            lmsControl->SetFrequencySX(LMS7002M::Rx, 1e6);
            lmsControl->Modify_SPI_Reg_bits(LML1_MODE, 0);
            lmsControl->Modify_SPI_Reg_bits(LML2_MODE, 0);
            lmsControl->Modify_SPI_Reg_bits(PD_RX_AFE2, 0);

            lmsControl->SetActiveChannel(LMS7002M::ChAB);
            lmsControl->Modify_SPI_Reg_bits(INSEL_RXTSP, 1);
            lmsControl->Modify_SPI_Reg_bits(GFIR1_BYP_RXTSP, 1);
            lmsControl->Modify_SPI_Reg_bits(GFIR2_BYP_RXTSP, 1);
            lmsControl->Modify_SPI_Reg_bits(GFIR3_BYP_RXTSP, 1);
            lmsControl->Modify_SPI_Reg_bits(AGC_BYP_RXTSP, 1);
            lmsControl->Modify_SPI_Reg_bits(CMIX_BYP_RXTSP, 1);

            lmsControl->SetActiveChannel(LMS7002M::ChA);
            lmsControl->Modify_SPI_Reg_bits(TSGFCW_RXTSP, 1);
            lmsControl->Modify_SPI_Reg_bits(TSGFC_RXTSP, 1);
            lmsControl->SetActiveChannel(LMS7002M::ChB);
            lmsControl->Modify_SPI_Reg_bits(TSGFCW_RXTSP, 1);
            lmsControl->Modify_SPI_Reg_bits(TSGFC_RXTSP, 0);
            lmsControl->SetActiveChannel(LMS7002M::ChA);

            lmsControl->SetInterfaceFrequency(cgenFreq, 0, decimation);
        }

        auto txRate = lmsControl->GetSampleRate(LMS7002M::Tx, LMS7002M::Channel::ChA);
        auto rxRate = lmsControl->GetSampleRate(LMS7002M::Rx, LMS7002M::Channel::ChA);
        serPort->UpdateExternalDataRate(0, txRate, rxRate);
        printf("Sampling rates - Tx : %g MHz\t Rx : %g MHz\n", txRate/1e6, rxRate/1e6);

        //setup streaming
        const int chCount = 2;
        size_t streamId;
        StreamConfig config;
        config.channels.push_back(0);
        config.channels.push_back(1);
        config.isTx = false;
        config.format = StreamConfig::StreamDataFormat::STREAM_12_BIT_IN_16;
        config.linkFormat = StreamConfig::StreamDataFormat::STREAM_12_BIT_COMPRESSED;

        LMS_SamplesFIFO *mRxFIFO;
        LMS_SamplesFIFO *mTxFIFO;
        mRxFIFO = new LMS_SamplesFIFO(1, 1);
        mTxFIFO = new LMS_SamplesFIFO(1, 1);

        mRxFIFO->Reset(2*4096, chCount);
        mTxFIFO->Reset(2*4096, chCount);
        fpga::StopStreaming(serPort);

        fpga::InitializeStreaming(serPort, config);
        ResetUSBFIFO(dynamic_cast<LMS64CProtocol *>(serPort));

        fpga::StartStreaming(serPort);

        std::atomic<bool> stop;
        stop.store(false);

        StreamerLTE_ThreadData threadRxArgs;
        threadRxArgs.dataPort = serPort;
        threadRxArgs.FIFO = mRxFIFO;
        threadRxArgs.terminate = &stop;
        threadRxArgs.dataRate_Bps = nullptr;
        threadRxArgs.getCmd = nullptr;
        threadRxArgs.report = nullptr;


        std::thread threadRx = std::thread(ReceivePackets, threadRxArgs);


        complex16_t** buffers = new complex16_t*[chCount];
        const int streamsize = 680;//serPort->GetStreamSize(streamId);
        for(int i=0; i<chCount; ++i)
            buffers[i] = new complex16_t[streamsize*2];

        //Both channels are set to receive test signal
        //A channel full scale, B channel -6 dB
        //A channel amplitude should always be bigger than channel B
        //otherwise it would show channels data swapping
        //this_thread::sleep_for(chrono::seconds(3));
        bool samplesValid = true;
        bool channelsSwapped = false;
        int sampleIndex = 0;
        //int samplesRead = serPort->ReadStream(streamId, (void * const *)buffers, streamsize, 1000, metadata);
        uint64_t timestamp;
        int samplesRead = mRxFIFO->pop_samples(buffers, streamsize, chCount, &timestamp, 250);

    #ifdef DRAW_GNUPLOT
        GNUPlotPipe gp(true);
        gp.flush();
        //gp.write("set terminal wxt size 1300,800\n");
        gp.write("set terminal x11\n");
        gp.write("set multiplot layout 2,2\n");
        gp.write("unset xlabel\n");
        gp.write("unset ylabel\n");
    #endif
        for(int cnt=0; cnt<readCount; ++cnt)
        {
            //samplesRead = serPort->ReadStream(streamId, (void * const *)buffers, streamsize, 250, metadata);
            samplesRead = mRxFIFO->pop_samples(buffers, streamsize, chCount, &timestamp, 250);
            if(samplesRead == 0)
            {
                ++failuresToGetData;
                break;
            }
            for(int i=0; i < samplesRead; ++i)
            {
                //compare amplitudes from each channel, should be A > B
                float ampA = pow(buffers[0][i].i, 2) + pow(buffers[0][i].q, 2);
                float ampB = pow(buffers[1][i].i, 2) + pow(buffers[1][i].q, 2);
                //make sure both channels are receiving valid samples
                if(ampA == 0 || ampB == 0)
                {
                    samplesValid = false;
                    printf("AmpA : %g \t AmpB: %g, @ %i\n", ampA, ampB, sampleIndex);
                    break;
                }
                if(ampA < ampB)
                {
                    channelsSwapped = true;
                    printf("AmpA : %g \t AmpB: %g @ %i\n", ampA, ampB, sampleIndex);
                    printf("samplesA : %i %i \t samplesB: %i %i\n", buffers[0][i].i, buffers[0][i].q, buffers[1][i].i, buffers[1][i].q);
                    break;
                }
                ++sampleIndex;
            }
    #ifdef DRAW_GNUPLOT
            for(int ch = 0; ch < chCount && (channelsSwapped || !samplesValid); ++ch)
            {
                if(ch == 0)
                    gp.write("set tmargin at screen 0.90; set bmargin at screen 0.60\n");
                else
                    gp.write("set tmargin at screen 0.40; set bmargin at screen 0.10\n");

                gp.write("set lmargin at screen 0.10; set rmargin at screen 0.70\n");

                const int sToDraw = samplesRead;

                gp.write("set size nosquare\n");
                gp.writef("set xrange [0:%i]\n", sToDraw);
                gp.write("set xtics\n");
                gp.write("set ytics\n");
                gp.writef("set title 'channel %i'\n", ch);
                gp.write("plot '-' with lines title 'I', '-' with lines title 'Q'\n");
                for(int s=0; s<sToDraw; ++s)
                    gp.writef("%i\n", buffers[ch][s].i);
                gp.write("e\n");
                for(int s=0; s<sToDraw; ++s)
                    gp.writef("%i\n", buffers[ch][s].q);
                gp.write("e\n");
                gp.write("set size square\n");
                gp.write("set yrange [-2050:2050]\n");
                gp.write("set xrange [-2050:2050]\n");
                gp.write("unset xtics\n");
                gp.write("unset ytics\n");
                gp.write("set lmargin at screen 0.70; set rmargin at screen 0.95\n");
                gp.write("plot '-' u 1:2 with lines notitle\n");
                for(int s=0; s<sToDraw; ++s)
                    gp.writef("%i %i\n", buffers[ch][s].i, buffers[ch][s].q);
                gp.write("e\n");
            }
            gp.write("unset multiplot\n");
            gp.flush();
#endif
            if(!samplesValid || channelsSwapped)
            {
                if(samplesValid)
                    ++invalidSamplesCount;
                else if(channelsSwapped)
                    ++swapCount;
                printf("Run %i @ read %i - samples valid: %s,  channels swapped: %s\n", c, cnt, samplesValid ? "Y" : "N", channelsSwapped ? "Y" : "N");
                break;
            }
        }
        if(samplesValid && !channelsSwapped && samplesRead!=0)
            ++successCount;
        stop.store(true);
        threadRx.join();
        fpga::StopStreaming(serPort);
//        ok = serPort->ControlStream(streamId, false, 680, metadata);
//        serPort->CloseStream(streamId);
        for(int i=0; i<chCount; ++i)
            delete buffers[i];
        delete buffers;
        delete mRxFIFO;
    }
    ConnectionRegistry::freeConnection(serPort);

    printf("Packet reads in test : %i\n", tryCount);
    printf("Test attempts : %i\n", tryCount);
    printf("Correct reads : %i\n", successCount);
    printf("Failed reads  : %i\n", failuresToGetData);
    printf("Invalid samples : %i\n", invalidSamplesCount);
    printf("Channel swaps : %i\n", swapCount);


    return 0;
}


