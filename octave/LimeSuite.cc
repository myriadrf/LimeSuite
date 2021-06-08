#include <octave/oct.h>
#include <octave/Cell.h>

#include <vector>
#include <string>

#include "LimeSuite.h"

using namespace std;

void FreeResources();

const int maxChCnt = 2;
const float scaleFactor = 32768.0f;

lms_device_t* lmsDev = NULL;
lms_stream_t streamRx[maxChCnt];
lms_stream_t streamTx[maxChCnt];

struct complex16_t
{
    int16_t i;
    int16_t q;
};

bool WFMrunning = false;
complex16_t* rxbuffers = NULL;
complex16_t* txbuffers = NULL;

void StopStream()
{
    if(lmsDev == NULL)
        return;
    for (int i = 0; i < maxChCnt; i++)
    {
        LMS_StopStream(&streamRx[i]);
        LMS_StopStream(&streamTx[i]);
        if (streamRx[i].handle)
            LMS_DestroyStream(lmsDev,&streamRx[i]);
        if (streamTx[i].handle)
            LMS_DestroyStream(lmsDev,&streamTx[i]);
        streamRx[i].handle = 0;
        streamTx[i].handle = 0;
    }
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
                      << " Protocol: " << info->protocolVersion
                      << " GW: " << info->gatewareVersion << endl;
    }
}

DEFUN_DLD (LimeGetDeviceList, args, nargout,
"LIST = LimeGetDeviceList() - Returns available device list")
{
    if (args.length ()!=0)
    {
        print_usage();
        return octave_value(-1);
    }
    lms_info_str_t dev_list[64];
    int devCount = LMS_GetDeviceList((lms_info_str_t*)&dev_list);
    dim_vector dim(devCount, 1);
    Cell c;
    octave_value_list retval;
    //devNames.resize(devCount, 0, string(""));
    for(int i=0; i<devCount; ++i)
    {
        c.insert(octave_value (dev_list[i], '"'),i,0);
        //retval(i) =  octave_value (dev_list[i], '"');
    }
    return octave_value(c);//retval;
}

DEFUN_DLD (LimeInitialize, args, nargout,
"LimeInitialize(DEV) - connect to device and allocate buffer memory\n\
DEV [optional] - device name to connect, obtained via LimeGetDeviceList()")
{
    FreeResources();
    int status;
    int nargin = args.length ();

    if (args.length ()>1)
    {
        print_usage();
        return octave_value(-1);
    }

    if(nargin > 0)
    {
        string deviceName = args(0).string_value();
        status = LMS_Open(&lmsDev, (char*)deviceName.c_str(), NULL);
    }
    else
        status = LMS_Open(&lmsDev, NULL, NULL);

    LMS_Synchronize(lmsDev,false);
    if(status != 0)
    {
        octave_stdout << LMS_GetLastErrorMessage() << endl;
        return octave_value(status);
    }
    PrintDeviceInfo(lmsDev);

    for (int i = 0; i < maxChCnt; i++)
    {
        streamRx[i].handle = 0;
        streamTx[i].handle = 0;
    }

    return octave_value(0);
}


DEFUN_DLD (LimeDestroy, args, nargout,
"LimeDestroy() - Stop all streams, deallocate memory and disconnect device")
{
    FreeResources();
    return octave_value_list();
}

DEFUN_DLD (LimeLoadConfig, args, nargout,
"LimeLoadConfig(FILENAME) - Load configuration from file FILENAME")
{
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }

    int nargin = args.length();

    if(nargin != 1)
    {
        print_usage();
        return octave_value(-1);
    }

    string filename = args(0).string_value();
    octave_stdout << "LimeLoadConfig loading: " << filename << endl;
    if (LMS_LoadConfig(lmsDev, filename.c_str())!=0)
    {
         octave_stdout << LMS_GetLastErrorMessage() << endl;
         return octave_value(-1);
    }
    octave_stdout << "Config loaded successfully: " << endl;

    return octave_value(0);
}

DEFUN_DLD (LimeStartStreaming, args, nargout,
"LimeStartStreaming(FIFOSIZE, CHANNELS) - starts sample streaming from selected channels\n\
 FIFOSIZE [optional] - buffer size in samples to be used by library (default: 4 MSamples)\
 CHANNELS [optional] - array of channels to be used [rx0 ; rx1 ; tx0 ; tx1] (deafult: rx0)")
{
    int nargin = args.length();
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }
    bool tx[maxChCnt] = {false};
    bool rx[maxChCnt] = {false};
    int fifoSize = 4*1024*1024; //4MS
    int channels = 1;

    if ((nargin > 2))
    {
        print_usage();
        return octave_value(-1);
    }

    if(nargin > 0 && args(0).int_value() > 0)
        fifoSize = args(0).int_value();

    if (nargin == 2)
    {
        int rowCnt = args(1).char_matrix_value().rows();;
        rowCnt = rowCnt < maxChCnt*2 ? rowCnt : maxChCnt*2;
        for (int i = 0 ; i < rowCnt ; i++)
        {
            std::string entry = args(1).char_matrix_value().row_as_string(i);
            if (entry == "rx0" || entry == "rx")
                rx[0] = true;
            else if (entry == "rx1")
                rx[1] = true;
            else if (entry == "tx0" || entry == "tx")
                tx[0] = true;
            else if (entry == "tx1")
                tx[1] = true;
            else
            {
                octave_stdout << "Invalid channel parameter" << endl;
                return octave_value(-1);
            }
        }
    }
    else
        rx[0] = true;

    for (int i = 0; i < maxChCnt; i++)
    {
        if (rx[i])
        {
            streamRx[i].channel = i;
            streamRx[i].fifoSize = fifoSize;
            streamRx[i].dataFmt = lms_stream_t::LMS_FMT_I16;
            streamRx[i].isTx = false;
            streamRx[i].throughputVsLatency = 0.5;
            if(LMS_SetupStream(lmsDev, &streamRx[i]) != 0)
                octave_stdout << LMS_GetLastErrorMessage() << endl;
        }
        if (tx[i])
        {
            streamTx[i].channel = i;
            streamTx[i].fifoSize = fifoSize;
            streamTx[i].dataFmt = lms_stream_t::LMS_FMT_I16;
            streamTx[i].isTx = true;
            streamTx[i].throughputVsLatency = 0.5;
            if(LMS_SetupStream(lmsDev, &streamTx[i]) != 0)
                octave_stdout << LMS_GetLastErrorMessage() << endl;
        }
    }

    for (int i = 0; i < maxChCnt; i++)
    {
        if (rx[i])
        {
            if (!rxbuffers)
                rxbuffers = new complex16_t[fifoSize/2];
            if(LMS_StartStream(&streamRx[i]) != 0)
                octave_stdout << LMS_GetLastErrorMessage() << endl;
        }
        if (tx[i])
        {
            if (!txbuffers)
                txbuffers = new complex16_t[fifoSize/2];
            if(LMS_StartStream(&streamTx[i]) != 0)
                octave_stdout << LMS_GetLastErrorMessage() << endl;
        }
    }

    return octave_value_list();
}

DEFUN_DLD (LimeStopStreaming, args, nargout,
"LimeStopStreaming() - Stop Receiver and Transmitter threads")
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

DEFUN_DLD (LimeReceiveSamples, args, ,
"SIGNAL = LimeReceiveSamplesLarge( N, CH) - receive N samples from Rx channel CH.\n\
CH parameter is optional, valid values are 0 and 1")
{
    if (!rxbuffers)
    {
        octave_stdout << "Rx streaming not initialized" << endl;
        return octave_value(-1);
    }

    int nargin = args.length ();
    if (nargin != 2 && nargin != 1)
    {
        print_usage ();
        return octave_value_list ();
    }
    const int samplesToReceive = args(0).int_value ();

    unsigned chIndex;
    if (nargin == 2)
    {
        chIndex = args(1).int_value ();
        if (chIndex >= maxChCnt)
        {
            octave_stdout << "Invalid channel number" << endl;
            return octave_value(-1);
        }
    }
    else
    {
        for (chIndex = 0; chIndex < maxChCnt; chIndex++)
            if (streamRx[chIndex].handle != 0)
                break;
    }

    Complex val=Complex(0.0,0.0);
    ComplexRowVector iqdata( samplesToReceive, val ); // index 0 to N-1

    const int timeout_ms = 1000;
    lms_stream_meta_t meta;
    int samplesCollected = 0;
    int retries = 5;
    while(samplesCollected < samplesToReceive && retries--)
    {
        int samplesToRead = 0;
        if(samplesToReceive-samplesCollected > streamRx[chIndex].fifoSize/2)
            samplesToRead = streamRx[chIndex].fifoSize/2;
        else
            samplesToRead = samplesToReceive-samplesCollected;
        int samplesRead = LMS_RecvStream(&streamRx[chIndex], (void*)rxbuffers, samplesToRead, &meta, timeout_ms);
        if (samplesRead < 0)
        {
            octave_stdout << "Error reading samples" << endl;
            return octave_value(-1);
        }
        for(int i=0; i<samplesRead; ++i)
        {
            iqdata(samplesCollected+i)=Complex(rxbuffers[i].i/scaleFactor,rxbuffers[i].q/scaleFactor);
        }
        samplesCollected += samplesRead;
    }
    return octave_value(iqdata);
}

DEFUN_DLD (LimeTransmitSamples, args, ,
"LimeTransmitSamples( SIGNAL, CH) - sends normalized complex SIGNAL to Tx cahnnel CH\n\
CH parameter is optional, valid values are 0 and 1")
{
    if (!txbuffers)
    {
        octave_stdout << "Tx streaming not initialized" << endl;
        return octave_value(-1);
    }

    int nargin = args.length ();
    int check=0;
    if (nargin != 2 && nargin != 1)
    {
        print_usage ();
        return octave_value_list ();
    }

    int chIndex = 0;
    if (nargin == 2)
    {
        chIndex = args(1).int_value ();
        if (chIndex >= maxChCnt)
        {
            octave_stdout << "Invalid channel number" << endl;
            return octave_value(-1);
        }
    }
    else
    {
        for (chIndex = 0; chIndex < maxChCnt; chIndex++)
            if (streamTx[chIndex].handle != 0)
                break;
    }

    ComplexRowVector    iqdata=args(0).complex_row_vector_value();
    dim_vector          iqdataSize=iqdata.dims();
    int samplesCount = iqdataSize(0) > iqdataSize(1) ? iqdataSize(0) : iqdataSize(1);

    for(int i=0; i < samplesCount; ++i)
    {
        octave_value    iqdatum = scaleFactor*iqdata(i);
        Complex iqdatum2 = iqdatum.complex_value();
        short i_sample = iqdatum2.real(); //
        short q_sample = iqdatum2.imag(); //
        txbuffers[i].i = i_sample;
        txbuffers[i].q = q_sample;
    }

    const int timeout_ms = 1000;
    lms_stream_meta_t meta;
    meta.waitForTimestamp = false;
    meta.timestamp = 0;
    int samplesWrite = samplesCount;
    samplesWrite = LMS_SendStream(&streamTx[chIndex], (const void*)txbuffers, samplesCount, &meta, timeout_ms);

    return octave_value (samplesWrite);
}

DEFUN_DLD (LimeLoopWFMStart, args, ,
"LimeLoopWFMStart(SIGNAL) - upload SIGNAL to device RAM for repeated transmitting")
{
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }
    int nargin = args.length ();
    int check=0;
    if (nargin != 2 && nargin != 1)
    {
        print_usage ();
        return octave_value_list ();
    }

    const int chCount = nargin;
    const int timeout_ms = 1000;

    ComplexRowVector    iqdata=args(0).complex_row_vector_value();
    dim_vector          iqdataSize=iqdata.dims();
    int samplesCount = iqdataSize(0) > iqdataSize(1) ? iqdataSize(0) : iqdataSize(1);
    int wfmLength = samplesCount;

    complex16_t **wfmBuffers = new complex16_t*[chCount];
    for(int i=0; i<chCount; ++i)
        wfmBuffers[i] = new complex16_t[samplesCount];

    for (int ch = 0; ch < chCount; ch++)
        for(int i=0; i < samplesCount; ++i)
        {
            octave_value    iqdatum = 2047*iqdata(i);
            Complex iqdatum2 = iqdatum.complex_value();
            float i_sample = iqdatum2.real(); //
            float q_sample = iqdatum2.imag(); //
            wfmBuffers[ch][i].i = i_sample;
            wfmBuffers[ch][i].q = q_sample;
        }

    LMS_UploadWFM(lmsDev, (const void**)wfmBuffers, chCount, samplesCount, 0);
    LMS_EnableTxWFM(lmsDev, 0, true);
    for(int i=0; i<chCount; ++i)
        delete wfmBuffers[i];
    delete wfmBuffers;
    WFMrunning = true;
    return octave_value_list();
}

DEFUN_DLD (LimeLoopWFMStop, args, ,
"LimeTxLoopWFMStop() - stop transmitting samples from device RAM")
{
    if(lmsDev == NULL)
    {
        octave_stdout << "LimeSuite not initialized" << endl;
        return octave_value(-1);
    }
    if(WFMrunning)
        LMS_EnableTxWFM(lmsDev, 0, false);
    WFMrunning = false;
    return octave_value ();
}

void FreeResources()
{
    if(lmsDev)
    {
        if(WFMrunning)
            LMS_EnableTxWFM(lmsDev, 0, false);
        StopStream();
        LMS_Close(lmsDev);
        lmsDev = NULL;
    }
    if(rxbuffers)
    {
        delete rxbuffers;
        rxbuffers = NULL;
    }
    if(txbuffers)
    {
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
