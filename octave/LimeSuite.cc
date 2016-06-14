#include <octave/oct.h>

#include <vector>
#include <string>

#include "LimeSuite.h"

using namespace std;

void FreeResources();

lms_device_t* lmsDev = NULL;

struct complex16_t
{
    int16_t i;
    int16_t q;
};

bool WFMrunning = false;
const int chCount = 1;
const int samplesMaxCount = 16*1360/chCount;
complex16_t** rxbuffers = NULL;
complex16_t** txbuffers = NULL;

void StopStream()
{
    if(lmsDev == NULL)
        return;
    LMS_StopStream(lmsDev, false);
    LMS_StopStream(lmsDev, true);
}

void PrintDeviceInfo(lms_device_t* port)
{
    if(port == NULL || lmsDev == NULL)
        return;
    const lms_dev_info_t* info = LMS_GetDeviceInfo(lmsDev);
    if(info == NULL)
        octave_stdout << "Failed to get device info" << endl;
    else
    {
        octave_stdout << "Connected to device: " << info->deviceName
                      << " FW: " << info->firmwareVersion << " HW: " << info->hardwareVersion
                      << " Protocol: " << info->protocolVersion << " GW: " << info->gatewareVersion
                      << " GW_rev: " << info->gatewareRevision << endl;
    }
}

DEFUN_DLD (LimeGetDeviceList, args, nargout,
           "Returns available device list")
{
    lms_info_str_t dev_list[64];
    int devCount = LMS_GetDeviceList((lms_info_str_t*)&dev_list);
    dim_vector dim(devCount, 1);
    Array<string> devNames(dim);
    //devNames.resize(devCount, 0, string(""));
    for(int i=0; i<devCount; ++i)
    {
        devNames(i) = dev_list[i];
    }
    return octave_value(devNames);
}

DEFUN_DLD (LimeInitialize, args, nargout,
           "Lime Suite initialization, connects device and allocates buffer memory")
{
    FreeResources();
    int status;
    int nargin = args.length ();
    if(nargin > 0)
    {
        string deviceName = args(0).string_value();
        status = LMS_Open(&lmsDev, (char*)deviceName.c_str(), NULL);
    }
    else
        status = LMS_Open(&lmsDev, NULL, NULL);
    if(status != 0)
    {
        octave_stdout << LMS_GetLastErrorMessage() << endl;
        return octave_value(-1);
    }
    PrintDeviceInfo(lmsDev);

    octave_stdout << "LimeInitialize has "
                  << nargin << " input arguments and "
                  << nargout << " output arguments.\n";

    rxbuffers = new complex16_t*[chCount];
    for(int i=0; i<chCount; ++i)
        rxbuffers[i] = new complex16_t[samplesMaxCount];

    txbuffers = new complex16_t*[chCount];
    for(int i=0; i<chCount; ++i)
        txbuffers[i] = new complex16_t[samplesMaxCount];

    return octave_value(0);
}

DEFUN_DLD (LimeGetDeviceInfo, args, nargout,
           "Prints out currently connected device info")
{
    PrintDeviceInfo(lmsDev);
    return octave_value_list();
}


DEFUN_DLD (LimeDestroy, args, nargout,
           "Stops all streams, deallocates memory and disconnects device")
{
    FreeResources();
    return octave_value_list();
}

DEFUN_DLD (LimeLoadConfig, args, nargout,
           "Load configuration file")
{
    int nargin = args.length();

    if(nargin < 1)
    {
        octave_stdout << "LoadConfig: requires filename" << endl;
        return octave_value_list();
    }
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }

    string filename = args(0).string_value();
    octave_stdout << "LimeLoadConfig loading: " << filename << endl;
    LMS_LoadConfig(lmsDev, filename.c_str());
    float_type cgenFreq;
    LMS_GetClockFreq(lmsDev, LMS_CLOCK_CGEN, &cgenFreq);
    LMS_SetClockFreq(lmsDev, LMS_CLOCK_CGEN, cgenFreq);
    octave_stdout << "Config loaded successfully: " << endl;

    return octave_value(0);
}

DEFUN_DLD (LimeStartStreaming, args, nargout,
           "Starts Receiver and Transmitter threads, first parameter internal buffer length in samples")
{
    int nargin = args.length();
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }

    lms_stream_conf_t streamConfig;
    if(nargin > 0)
        streamConfig.fifoSize = args(0).int_value();
    else
        streamConfig.fifoSize = 1360*64;
    streamConfig.transferSize = 65536;
    streamConfig.numTransfers = 16;
    streamConfig.channels = 1;
    streamConfig.dataFmt = lms_stream_conf_t::LMS_FMT_I16;
    streamConfig.linkFmt = lms_stream_conf_t::LMS_LINK_12BIT;

    if(LMS_SetupStream(lmsDev, streamConfig) != 0)
        octave_stdout << LMS_GetLastErrorMessage() << endl;

    if(LMS_StartStream(lmsDev, false) != 0)
        octave_stdout << LMS_GetLastErrorMessage() << endl;
    if(LMS_StartStream(lmsDev, true) != 0)
        octave_stdout << LMS_GetLastErrorMessage() << endl;
    return octave_value_list();
}

DEFUN_DLD (LimeStopStreaming, args, nargout,
           "Stops Receiver and Transmitter threads")
{
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }
    int nargin = args.length();
    octave_stdout << "StopStreaming" << endl;
    StopStream();
    return octave_value_list();
}

DEFUN_DLD (LimeReceiveSamples, args, , "wfm = LimeReceiveSamplesLarge( samplesCount ), performs long read, returning one large buffer")
{
    int nargin = args.length ();
    if (nargin != 1)
    {
        print_usage ();
        return octave_value_list ();
    }
    const int samplesToReceive = args(0).int_value ();

    Complex val=Complex(0.0,0.0);
    ComplexRowVector iqdata( samplesToReceive, val ); // index 0 to N-1

    const int timeout_ms = 1000;
    lms_stream_meta_t meta;
    int samplesCollected = 0;
    
    while(samplesCollected < samplesToReceive)
    {
        int samplesToRead = 0;
        if(samplesToReceive-samplesCollected > samplesMaxCount)
            samplesToRead = samplesMaxCount;
        else
            samplesToRead = samplesToReceive-samplesCollected;
        int samplesRead = LMS_RecvStream(lmsDev, (void**)rxbuffers, samplesToRead, &meta, timeout_ms);
        for(int i=0; i<samplesRead; ++i)
        {
            iqdata(samplesCollected+i)=Complex(rxbuffers[0][i].i/2048.0,rxbuffers[0][i].q/2048.0);
        }
        samplesCollected += samplesRead;
    }
    return octave_value(iqdata);
}

DEFUN_DLD (LimeTransmitSamples, args, , "LimeTransmitSamples( wfm, timestamp)")
{
    int nargin = args.length ();
    int check=0;
    if (nargin != 1)
    {
        print_usage ();
        return octave_value_list ();
    }

    ComplexRowVector    iqdata=args(0).complex_row_vector_value();
    dim_vector          iqdataSize=iqdata.dims();
    int samplesCount = iqdataSize(0) > iqdataSize(1) ? iqdataSize(0) : iqdataSize(1);

    for(int i=0; i < samplesCount; ++i)
    {
        octave_value    iqdatum = 2047*iqdata(i);
        Complex iqdatum2 = iqdatum.complex_value();
        short i_sample = iqdatum2.real(); //
        short q_sample = iqdatum2.imag(); //
        txbuffers[0][i].i = i_sample;
        txbuffers[0][i].q = q_sample;
    }

    const int timeout_ms = 1000;
    lms_stream_meta_t meta;
    meta.has_timestamp = false;
    meta.timestamp = 0;
    int samplesWrite = samplesCount;
    samplesWrite = LMS_SendStream(lmsDev, (const void**)txbuffers, samplesCount, &meta, timeout_ms);

    return octave_value ();
}

DEFUN_DLD (LimeLoopWFMStart, args, , "LimeTxLoopWFMStart (wfm)")
{
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }
    int nargin = args.length ();
    int check=0;
    if (nargin != 1)
    {
        print_usage ();
        return octave_value_list ();
    }

    const int timeout_ms = 1000;

    ComplexRowVector    iqdata=args(0).complex_row_vector_value();
    dim_vector          iqdataSize=iqdata.dims();
    int samplesCount = iqdataSize(0) > iqdataSize(1) ? iqdataSize(0) : iqdataSize(1);
    int wfmLength = samplesCount;

    complex16_t **wfmBuffers = new complex16_t*[chCount];
    for(int i=0; i<chCount; ++i)
        wfmBuffers[i] = new complex16_t[samplesCount];

    for(int i=0; i < samplesCount; ++i)
    {
        octave_value    iqdatum = 2047*iqdata(i);
        Complex iqdatum2 = iqdatum.complex_value();
        float i_sample = iqdatum2.real(); //
        float q_sample = iqdatum2.imag(); //
        wfmBuffers[0][i].i = i_sample;
        wfmBuffers[0][i].q = q_sample;
    }

    LMS_StreamStartLoopWFM(lmsDev, (const void**)wfmBuffers, samplesCount);

    for(int i=0; i<chCount; ++i)
        delete wfmBuffers[i];
    delete wfmBuffers;
    WFMrunning = true;
    return octave_value_list();
}

DEFUN_DLD (LimeLoopWFMStop, args, , "LimeTxLoopWFMStop")
{
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }
    if(WFMrunning)
        LMS_StreamStopLoopWFM(lmsDev);
    WFMrunning = false;
    return octave_value ();
}

void FreeResources()
{
    if(lmsDev)
    {
        if(WFMrunning)
            LMS_StreamStopLoopWFM(lmsDev);
        StopStream();
        LMS_DestroyStream(lmsDev);
        LMS_Close(lmsDev);
        lmsDev = NULL;
    }
    if(rxbuffers)
    {
        for(int i=0; i<chCount; ++i)
            delete rxbuffers[i];
        delete rxbuffers;
        rxbuffers = NULL;
    }
    if(txbuffers)
    {
        for(int i=0; i<chCount; ++i)
            delete txbuffers[i];
        delete txbuffers;
        txbuffers = NULL;
    }
}

class ResourceDeallocator
{
public:
    ResourceDeallocator() {};
    ~ResourceDeallocator()
    {
        FreeResources();
    };
};

ResourceDeallocator gResources;
