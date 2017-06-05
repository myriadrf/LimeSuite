#include "IConnection.h"
#include "ConnectionRegistry.h"
#include "lime/LimeSuite.h"
#include "lms7_device.h"
#include "ErrorReporting.h"
#include "errno.h"
#include "MCU_BD.h"
#include <cmath>
#include "VersionInfo.h"
#include <assert.h>
#include "FPGA_common.h"

using namespace std;

API_EXPORT int CALL_CONV LMS_GetDeviceList(lms_info_str_t * dev_list)
{
    std::vector<lime::ConnectionHandle> handles;
    handles = lime::ConnectionRegistry::findConnections();

    if (dev_list != nullptr)
    {
        for (size_t i = 0; i < handles.size(); i++)
        {
            string str = handles[i].serialize();
            if (dev_list[i] == nullptr)
                return -1;
            strncpy(dev_list[i],str.c_str(),sizeof(lms_info_str_t)-1);
            dev_list[i][sizeof(lms_info_str_t)-1]=0;
        }
    }
    return handles.size();
}

API_EXPORT int CALL_CONV LMS_Open(lms_device_t** device, const lms_info_str_t info, void* args)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device pointer cannot be NULL");
        return -1;
    }

    std::vector<lime::ConnectionHandle> handles;
    handles = lime::ConnectionRegistry::findConnections();
    LMS7_Device* lms = (LMS7_Device*)*device;
    if (lms != nullptr)
        lms->SetConnection(nullptr);

    for (size_t i = 0; i < handles.size(); i++)
    {
        if (info == NULL || strcmp(handles[i].serialize().c_str(),info) == 0)
        {
            auto conn = lime::ConnectionRegistry::makeConnection(handles[i]);
            if (conn->IsOpen() == false)
            {
                lime::ConnectionRegistry::freeConnection(conn);
                if (info != NULL)
                {
                    lime::ReportError(EBUSY, "Failed to open. Device is busy.");
                    return -1;
                }
                else
                    continue;
            }
            *device = LMS7_Device::CreateDevice(conn,lms);
            return LMS_SUCCESS;
        }
    }

    if (info == NULL)
    {
        *device = LMS7_Device::CreateDevice(nullptr);
        return LMS_SUCCESS;
    }

    lime::ReportError(ENODEV, "Specified device could not be found");
    return -1;
}

API_EXPORT int CALL_CONV LMS_Close(lms_device_t * device)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;
    delete lms;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_Disconnect(lms_device_t *device)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->SetConnection(nullptr);
    }

API_EXPORT bool CALL_CONV LMS_IsOpen(lms_device_t *device, int port)
{
    if (device == nullptr)
        return false;

    LMS7_Device* lms = (LMS7_Device*)device;

        auto conn = lms->GetConnection();
        if (conn != nullptr)
        {
            return conn->IsOpen();
        }
    return false;
}

API_EXPORT int CALL_CONV LMS_Reset(lms_device_t *device)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->Reset();
}

API_EXPORT int CALL_CONV LMS_EnableChannel(lms_device_t * device, bool dir_tx, size_t chan, bool enabled)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    return lms->EnableChannel(dir_tx, chan,enabled);
}

API_EXPORT int CALL_CONV LMS_SetSampleRate(lms_device_t * device, float_type rate, size_t oversample)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

   LMS7_Device* lms = (LMS7_Device*)device;

   return lms->SetRate(rate, oversample);
}

API_EXPORT int CALL_CONV LMS_SetSampleRateDir(lms_device_t *device, bool dir_tx, float_type rate, size_t oversample)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    return lms->SetRate(dir_tx,rate,oversample);
}

API_EXPORT int CALL_CONV LMS_GetSampleRate(lms_device_t *device, bool dir_tx, size_t chan, float_type *host_Hz, float_type *rf_Hz)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    double rate = lms->GetRate(dir_tx,chan,rf_Hz);
    if (host_Hz)
        *host_Hz = rate;

    return LMS_SUCCESS;

}

API_EXPORT int CALL_CONV LMS_GetSampleRateRange(lms_device_t *device, bool dir_tx, lms_range_t *range)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (dir_tx)
        *range = lms->GetRxRateRange();
    else
        *range = lms->GetTxRateRange();
    return LMS_SUCCESS;
}



API_EXPORT int CALL_CONV LMS_Init(lms_device_t * device)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    return lms->Init();
}

API_EXPORT int CALL_CONV LMS_ReadCustomBoardParam(lms_device_t *device,
                           uint8_t param_id, float_type *val, lms_name_t units)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;
    std::string str;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
        lime::ReportError(EINVAL, "Device not connected");
        return -1;
    }
    int ret=conn->CustomParameterRead(&param_id,val,1,&str);
    strncpy(units,str.c_str(),sizeof(lms_name_t)-1);
    return ret;
}

API_EXPORT int CALL_CONV LMS_WriteCustomBoardParam(lms_device_t *device,
                        uint8_t param_id, float_type val, const lms_name_t units)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
        lime::ReportError(EINVAL, "Device not connected");
        return -1;
    }
    std::string str = units == nullptr ? "":units;
    return conn->CustomParameterWrite(&param_id,&val,1,&str);
}

API_EXPORT int CALL_CONV LMS_VCTCXOWrite(lms_device_t * device, uint16_t val)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->DACWrite(val);
}

API_EXPORT int CALL_CONV LMS_VCTCXORead(lms_device_t * device, uint16_t *val)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;
    int ret = lms->DACRead();
    *val = ret;
    return ret < 0 ? -1 : 0;
}

API_EXPORT int CALL_CONV LMS_GetClockFreq(lms_device_t *device, size_t clk_id, float_type *freq)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;

    return lms->GetClockFreq(clk_id, freq);
}

API_EXPORT int CALL_CONV LMS_SetClockFreq(lms_device_t *device, size_t clk_id, float_type freq)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->SetClockFreq(clk_id, freq);
}

API_EXPORT  int CALL_CONV LMS_Synchronize(lms_device_t *dev, bool toChip)
{
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
     LMS7_Device* lms = (LMS7_Device*)dev;

     return lms->Synchronize(toChip);
}

API_EXPORT int CALL_CONV LMS_GPIORead(lms_device_t *dev,  uint8_t* buffer, size_t len)
{
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)dev;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
        lime::ReportError(EINVAL, "Device not connected");
        return -1;
    }
    return conn->GPIORead(buffer,len);
}

API_EXPORT int CALL_CONV LMS_GPIOWrite(lms_device_t *dev, const uint8_t* buffer, size_t len)
{
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
     LMS7_Device* lms = (LMS7_Device*)dev;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
       lime::ReportError(EINVAL, "Device not connected");
       return -1;
    }
     return conn->GPIOWrite(buffer,len);
}

API_EXPORT int CALL_CONV LMS_GPIODirRead(lms_device_t *dev,  uint8_t* buffer, size_t len)
{
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)dev;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
        lime::ReportError(EINVAL, "Device not connected");
        return -1;
    }
    return conn->GPIODirRead(buffer,len);
}

API_EXPORT int CALL_CONV LMS_GPIODirWrite(lms_device_t *dev, const uint8_t* buffer, size_t len)
{
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
     LMS7_Device* lms = (LMS7_Device*)dev;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
       lime::ReportError(EINVAL, "Device not connected");
       return -1;
    }
     return conn->GPIODirWrite(buffer,len);
}

API_EXPORT int CALL_CONV LMS_EnableCalibCache(lms_device_t *dev, bool enable)
{
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)dev;
    return lms->EnableCalibCache(enable);
}

API_EXPORT int CALL_CONV LMS_GetChipTemperature(lms_device_t *dev, size_t ind, float_type *temp)
{
    *temp = 0;
    if (dev == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)dev;
    uint16_t val;
    if (lms->ReadLMSReg(0x2F, &val))
    {
        return -1;
    }
    if (val == 0x3840)
    {
        lime::ReportError(EINVAL, "Feature is not available on this chip revision");
        return -1;
    }
    return lms->GetChipTemperature(ind,temp);
}

API_EXPORT int CALL_CONV LMS_GetNumChannels(lms_device_t * device, bool dir_tx)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->GetNumChannels(dir_tx);
}

API_EXPORT int CALL_CONV LMS_SetLOFrequency(lms_device_t *device, bool dir_tx, size_t chan, float_type frequency)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(false))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    if (dir_tx)
    {
        if (lms->SetTxFrequency(chan,frequency)!=0)
            return -1;
    }
    else
    {
        if (lms->SetRxFrequency(chan,frequency)!=0)
            return -1;
    }

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetLOFrequency(lms_device_t *device, bool dir_tx, size_t chan, float_type *frequency)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    *frequency = lms->GetTRXFrequency(dir_tx, chan);
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetLOFrequencyRange(lms_device_t *device, bool dir_tx, lms_range_t *range)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    *range = lms->GetFrequencyRange(dir_tx);
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetAntennaList(lms_device_t *device, bool dir_tx, size_t chan, lms_name_t *list)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(false))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    auto names = lms->GetPathNames(dir_tx, chan);
    if (list != nullptr)
        for (size_t i = 0; i < names.size(); i++)
        {
            strncpy(list[i], names[i].c_str(), sizeof(lms_name_t) - 1);
            list[i][sizeof(lms_name_t) - 1] = 0;
        }
    return names.size();
}

API_EXPORT int CALL_CONV LMS_SetAntenna(lms_device_t *device, bool dir_tx, size_t chan, size_t path)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(false))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    lms->SetPath(dir_tx,chan,path);

    return LMS_SUCCESS;
}


API_EXPORT int CALL_CONV LMS_GetAntenna(lms_device_t *device, bool dir_tx, size_t chan)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    return lms->GetPath(dir_tx, chan);
}

API_EXPORT int CALL_CONV LMS_GetAntennaBW(lms_device_t *device, bool dir_tx, size_t chan, size_t path, lms_range_t *range)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    if (dir_tx)
        *range = lms->GetTxPathBand(path,chan);
    else
        *range = lms->GetRxPathBand(path,chan);

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_SetLPFBW(lms_device_t *device, bool dir_tx, size_t chan, float_type bandwidth)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->SetLPF(dir_tx,chan,true,true,bandwidth);
}

API_EXPORT int CALL_CONV LMS_GetLPFBW(lms_device_t *device, bool dir_tx, size_t chan, float_type *bandwidth)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }
    *bandwidth = lms->GetLPFBW(dir_tx,chan,true);
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_SetLPF(lms_device_t *device, bool dir_tx, size_t chan, bool enabled)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }
    return lms->SetLPF(dir_tx,chan,true,enabled,1);
}

API_EXPORT int CALL_CONV LMS_SetGFIRLPF(lms_device_t *device, bool dir_tx, size_t chan, bool enabled, float_type bandwidth)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->SetLPF(dir_tx,chan,false,enabled,bandwidth);
}

API_EXPORT int CALL_CONV LMS_GetLPFBWRange(lms_device_t *device, bool dir_tx, lms_range_t *range)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    *range = lms->GetLPFRange(dir_tx,0,true);

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_SetNormalizedGain(lms_device_t *device, bool dir_tx, size_t chan, float_type gain)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    if (gain > 1.0)
        gain = 1.0;
    else if (gain < 0)
        gain = 0;

   return lms->SetNormalizedGain(dir_tx,chan,gain);
}

API_EXPORT int CALL_CONV LMS_SetGaindB(lms_device_t *device, bool dir_tx,
                                                size_t chan,unsigned gain)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

   return lms->SetGain(dir_tx,chan,gain);
}

API_EXPORT int CALL_CONV LMS_GetNormalizedGain(lms_device_t *device, bool dir_tx, size_t chan,float_type *gain)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    *gain = lms->GetNormalizedGain(dir_tx,chan);
    if (gain < 0)
        return -1;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetGaindB(lms_device_t *device, bool dir_tx, size_t chan, unsigned *gain)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    *gain = lms->GetGain(dir_tx,chan);
    if (gain < 0)
        return -1;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_Calibrate(lms_device_t *device, bool dir_tx, size_t chan, double bw, unsigned flags)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    uint16_t val;
    if (lms->ReadLMSReg(0x2F, &val))
    {
        return -1;
    }
    if (val == 0x3840)
    {
        lime::ReportError(EINVAL, "Feature is not available on this chip revision");
        return -1;
    }

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }
    return lms->Calibrate(dir_tx, chan, bw, flags);
}

API_EXPORT int CALL_CONV LMS_LoadConfig(lms_device_t *device, const char *filename)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    return lms->LoadConfig(filename);
}

API_EXPORT int CALL_CONV LMS_SaveConfig(lms_device_t *device, const char *filename)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    return lms->SaveConfig(filename);
}

API_EXPORT int CALL_CONV LMS_SetTestSignal(lms_device_t *device, bool dir_tx, size_t chan, lms_testsig_t sig, int16_t dc_i, int16_t dc_q)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    if (sig > LMS_TESTSIG_DC)
    {
        lime::ReportError(EINVAL, "Invalid signal.");
        return -1;
    }

    lms->SetTestSignal(dir_tx,chan,sig,dc_i,dc_q);

    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_GetTestSignal(lms_device_t *device, bool dir_tx, size_t chan, lms_testsig_t *sig)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    int tmp = lms->GetTestSignal(dir_tx,chan);
    if (tmp < 0)
        return -1;

    *sig = (lms_testsig_t)tmp;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_SetNCOFrequency(lms_device_t *device, bool dir_tx, size_t ch, const float_type *freq, float_type pho)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (ch >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->SetNCOFreq(dir_tx, ch,freq, pho);
}

API_EXPORT int CALL_CONV LMS_GetNCOFrequency(lms_device_t *device, bool dir_tx, size_t chan, float_type *freq, float_type *pho)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->GetNCOFreq(dir_tx,chan,freq,pho);
}

API_EXPORT int CALL_CONV LMS_SetNCOPhase(lms_device_t *device, bool dir_tx, size_t ch, const float_type *phase, float_type fcw)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (ch >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->SetNCOPhase(dir_tx, ch,phase, fcw);
}

API_EXPORT int CALL_CONV LMS_GetNCOPhase(lms_device_t *device, bool dir_tx, size_t ch, float_type *phase, float_type *fcw)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (ch >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->GetNCOPhase(dir_tx,ch,phase,fcw);
}

API_EXPORT int CALL_CONV LMS_SetNCOIndex(lms_device_t *device, bool dir_tx, size_t chan, int index,bool down)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->SetNCO(dir_tx,chan,index,down);
}

API_EXPORT int CALL_CONV LMS_GetNCOIndex(lms_device_t *device, bool dir_tx, size_t chan)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(dir_tx))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->GetNCO(dir_tx, chan);
}

API_EXPORT int CALL_CONV LMS_ReadLMSReg(lms_device_t *device, uint32_t address, uint16_t *val)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->ReadLMSReg(address, val);
}

API_EXPORT int CALL_CONV LMS_WriteLMSReg(lms_device_t *device, uint32_t address, uint16_t val)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->WriteLMSReg(address, val);
}

API_EXPORT int CALL_CONV LMS_ReadFPGAReg(lms_device_t *device, uint32_t address, uint16_t *val)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    uint32_t addr = address;
    uint32_t data;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
        lime::ReportError(EINVAL, "Device not connected");
        return -1;
    }
    *val = conn->ReadRegisters(&addr,&data,1);
    if (*val != LMS_SUCCESS)
        return *val;
    *val = data;
    return LMS_SUCCESS;
}

API_EXPORT int CALL_CONV LMS_WriteFPGAReg(lms_device_t *device, uint32_t address, uint16_t val)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
        lime::ReportError(EINVAL, "Device not connected");
        return -1;
    }
    return conn->WriteRegister(address,val);
}

API_EXPORT int CALL_CONV LMS_ReadParam(lms_device_t *device, struct LMS7Parameter param, uint16_t *val)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->ReadParam(param,val);
}

API_EXPORT int CALL_CONV LMS_WriteParam(lms_device_t *device, struct LMS7Parameter param, uint16_t val)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;

    return lms->WriteParam(param, val);
}

API_EXPORT int CALL_CONV LMS_SetGFIRCoeff(lms_device_t * device, bool dir_tx, size_t chan, lms_gfir_t filt, const float_type* coef,size_t count)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(false))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }
    return lms->SetGFIRCoef(dir_tx,chan,filt,coef,count);
}

API_EXPORT int CALL_CONV LMS_GetGFIRCoeff(lms_device_t * device, bool dir_tx, size_t chan, lms_gfir_t filt, float_type* coef)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(false))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }
    return lms->GetGFIRCoef(dir_tx,chan,filt,coef);
}

API_EXPORT int CALL_CONV LMS_SetGFIR(lms_device_t * device, bool dir_tx, size_t chan, lms_gfir_t filt, bool enabled)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }
    LMS7_Device* lms = (LMS7_Device*)device;

    if (chan >= lms->GetNumChannels(false))
    {
        lime::ReportError(EINVAL, "Invalid channel number.");
        return -1;
    }

    return lms->SetGFIR(dir_tx,chan,filt,enabled);
}

API_EXPORT int CALL_CONV LMS_SetupStream(lms_device_t *device, lms_stream_t *stream)
{
    if(device == nullptr)
        return lime::ReportError(EINVAL, "Device is NULL.");
    if(stream == nullptr)
        return lime::ReportError(EINVAL, "stream is NULL.");

    LMS7_Device* lms = (LMS7_Device*)device;

    lime::StreamConfig config;
    config.bufferLength = stream->fifoSize;
    config.channelID = stream->channel;
    config.performanceLatency = stream->throughputVsLatency;
    switch(stream->dataFmt)
    {
        case lms_stream_t::LMS_FMT_F32:
            config.format = lime::StreamConfig::STREAM_COMPLEX_FLOAT32;
            break;
        case lms_stream_t::LMS_FMT_I16:
            config.format = lime::StreamConfig::STREAM_12_BIT_IN_16;
            break;
        case lms_stream_t::LMS_FMT_I12:
            config.format = lime::StreamConfig::STREAM_12_BIT_COMPRESSED;
            break;
        default:
            config.format = lime::StreamConfig::STREAM_COMPLEX_FLOAT32;
    }
    config.isTx = stream->isTx;
    return lms->GetConnection(stream->channel)->SetupStream(stream->handle, config);
}

API_EXPORT int CALL_CONV LMS_DestroyStream(lms_device_t *device, lms_stream_t *stream)
{
    if(stream == nullptr)
        return lime::ReportError(EINVAL, "stream is NULL.");

    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->GetConnection(stream->channel)->CloseStream(stream->handle);
}

API_EXPORT int CALL_CONV LMS_StartStream(lms_stream_t *stream)
{
    if (stream==nullptr || stream->handle==0)
        return 0;
    return reinterpret_cast<lime::IStreamChannel*>(stream->handle)->Start();
}

API_EXPORT int CALL_CONV LMS_StopStream(lms_stream_t *stream)
{
    if (stream==nullptr || stream->handle==0)
        return 0;
    return reinterpret_cast<lime::IStreamChannel*>(stream->handle)->Stop();
}

API_EXPORT int CALL_CONV LMS_RecvStream(lms_stream_t *stream, void *samples, size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms)
{
    if (stream==nullptr || stream->handle==0)
        return -1;
    lime::IStreamChannel* channel = (lime::IStreamChannel*)stream->handle;
    lime::IStreamChannel::Metadata metadata;
    metadata.flags = 0;
    if (meta)
    {
        metadata.flags |= meta->waitForTimestamp * lime::IStreamChannel::Metadata::SYNC_TIMESTAMP;
        metadata.timestamp = meta->timestamp;
    }
    else metadata.timestamp = 0;

    int status = channel->Read(samples, sample_count, &metadata, timeout_ms);
    if (meta)
        meta->timestamp = metadata.timestamp;
    return status;
}

API_EXPORT int CALL_CONV LMS_SendStream(lms_stream_t *stream, const void *samples, size_t sample_count, const lms_stream_meta_t *meta, unsigned timeout_ms)
{
    if (stream==nullptr || stream->handle==0)
        return -1;
    lime::IStreamChannel* channel = (lime::IStreamChannel*)stream->handle;
    lime::IStreamChannel::Metadata metadata;
    metadata.flags = 0;
    if (meta)
    {
        metadata.flags |= meta->waitForTimestamp * lime::IStreamChannel::Metadata::SYNC_TIMESTAMP;
        metadata.timestamp = meta->timestamp;
    }
    else metadata.timestamp = 0;

    return channel->Write(samples, sample_count, &metadata, timeout_ms);
}

API_EXPORT int CALL_CONV LMS_UploadWFM(lms_device_t *device,
                                         const void **samples, uint8_t chCount,
                                         size_t sample_count, int format)
{
    LMS7_Device* lms = (LMS7_Device*)device;
    lime::StreamConfig::StreamDataFormat fmt;
    switch(format)
    {
        case 0:
            fmt = lime::StreamConfig::StreamDataFormat::STREAM_12_BIT_COMPRESSED;
            break;
        case 1:
            fmt = lime::StreamConfig::StreamDataFormat::STREAM_12_BIT_IN_16;
            break;
        case 2:
            fmt = lime::StreamConfig::StreamDataFormat::STREAM_COMPLEX_FLOAT32;
            break;
        default:
            fmt = lime::StreamConfig::StreamDataFormat::STREAM_12_BIT_COMPRESSED;
            break;
    }
    return lms->UploadWFM(samples, chCount, sample_count, fmt);
}

API_EXPORT int CALL_CONV LMS_EnableTxWFM(lms_device_t *device, unsigned ch, bool active)
{
    uint16_t regAddr = 0x000D;
    uint16_t regValue = 0;
    int status = 0;
    status = LMS_ReadFPGAReg(device, regAddr, &regValue);
    if(status != 0)
        return status;
    unsigned offset = (ch/2)*2;
    regValue = regValue & ~(0x6<<offset); //clear WFM_LOAD, WFM_PLAY
    regValue |= (active << (1+offset));
    status = LMS_WriteFPGAReg(device, regAddr, regValue);
    return status;
}

API_EXPORT int CALL_CONV LMS_GetStreamStatus(lms_stream_t *stream, lms_stream_status_t* status)
{
    assert(stream != nullptr);
    lime::IStreamChannel* channel = (lime::IStreamChannel*)stream->handle;
    if(channel == nullptr)
        return -1;
    lime::IStreamChannel::Info info = channel->GetInfo();

    status->active = false;
    status->droppedPackets = 0;
    status->fifoFilledCount = info.fifoItemsCount;
    status->fifoSize = info.fifoSize;
    status->linkRate = info.linkRate;
    status->overrun = 0;
    status->underrun = 0;
    status->sampleRate = 0;
    status->timestamp = 0;
    return 0;
}

API_EXPORT const lms_dev_info_t* CALL_CONV LMS_GetDeviceInfo(lms_device_t *device)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return nullptr;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    auto conn = lms->GetConnection();
    if (conn == nullptr)
    {
        lime::ReportError(EINVAL, "Device not connected");
        return nullptr;
    }
    if (!conn->IsOpen())
    {
       lime::ReportError(EINVAL, "No cennection to board.");
	   return nullptr;
    }

    return lms->GetInfo();
}

API_EXPORT int CALL_CONV LMS_Program(lms_device_t *device, const char *data, size_t size,
           lms_prog_trg_t component, lms_prog_md_t mode, lms_prog_callback_t callback)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->Program(data,size,component,mode, callback);
}



API_EXPORT int CALL_CONV LMS_ProgramUpdate(lms_device_t *device, const bool download, lms_prog_callback_t callback)
{
    if (device == nullptr)
    {
        lime::ReportError(EINVAL, "Device cannot be NULL.");
        return -1;
    }

    LMS7_Device* lms = (LMS7_Device*)device;
    return lms->ProgramUpdate(download,callback);
}

API_EXPORT const char * CALL_CONV LMS_GetLastErrorMessage(void)
{
    return lime::GetLastErrorMessage();
}

API_EXPORT const char* LMS_GetLibraryVersion()
{
    static char libraryVersion[32];
    sprintf(libraryVersion, "%.32s", lime::GetLibraryVersion().c_str());
    return libraryVersion;
}
