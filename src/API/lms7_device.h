/*
 * File:   lms7_device.h
 * Author: ignas
 *
 * Created on March 9, 2016, 12:54 PM
 */

#ifndef LMS7_DEVICE_H
#define	LMS7_DEVICE_H
#include "LMS7002M.h"
#include "lime/LimeSuite.h"
#include <mutex>
#include <vector>
#include <map>
#include <string>
#include <IConnection.h>

class LIME_API LMS7_Device
{
    class lms_channel_info
    {
    public:
        lms_channel_info()
        {
            lpf_bw = 5e6;
            cF_offset_nco = 0;
            sample_rate = 30e6;
        }
        float_type lpf_bw;
        float_type cF_offset_nco;
        float_type sample_rate;
    };
public:
    LMS7_Device(LMS7_Device *obj = nullptr);
    virtual ~LMS7_Device();
    virtual int SetConnection(lime::IConnection* conn);
    virtual lime::IConnection* GetConnection(unsigned chan =0);
    int Init();
    int EnableChannel(bool dir_tx, size_t chan, bool enabled);
    int Reset();
    int SetReferenceClock(const float_type refCLK_MHz);
    virtual size_t GetNumChannels(const bool tx=false) const;
    int SetRate(float_type f_MHz, int oversample);
    int SetRate(bool tx, float_type f_MHz, size_t oversample = 0);
    float_type GetRate(bool tx,size_t chan,float_type *rf_rate_Hz = NULL);
    lms_range_t GetRxRateRange(const size_t chan = 0)const;
    lms_range_t GetTxRateRange(const size_t chan = 0)const;
    std::vector<std::string> GetPathNames(bool dir_tx, size_t chan) const;
    int SetPath(bool tx,size_t chan, size_t path);
    size_t GetPath(bool tx, size_t chan);
    int SetRxFrequency(size_t chan, float_type f_Hz);
    int SetTxFrequency(size_t chan, float_type f_Hz);
    float_type GetTRXFrequency(bool tx, size_t chan);
    lms_range_t GetFrequencyRange(bool tx) const;
    lms_range_t GetRxPathBand(size_t path, size_t chan) const;
    lms_range_t GetTxPathBand(size_t path, size_t chan) const;
    int SetLPF(bool tx, size_t chan, bool f, bool en, float_type bandwidth=-1);
    float_type GetLPFBW(bool tx,size_t chan, bool filt);
    lms_range_t GetLPFRange(bool tx,size_t chan,bool f);
    int SetGFIRCoef(bool tx, size_t chan, lms_gfir_t filt, const float_type* coef,size_t count);
    int GetGFIRCoef(bool tx, size_t chan, lms_gfir_t filt, float_type* coef);
    int SetGFIR(bool tx, size_t chan, lms_gfir_t filt, bool enabled);
    int SetGain(bool dir_tx, size_t chan,unsigned gain);
    int GetGain(bool dir_tx, size_t chan);
    int SetNormalizedGain(bool dir_tx, size_t chan,float_type gain);
    float_type GetNormalizedGain(bool dir_tx, size_t chan);
    int SetTestSignal(bool dir_tx, size_t chan,lms_testsig_t sig,int16_t dc_i =0, int16_t dc_q = 0);
    int GetTestSignal(bool dir_tx, size_t chan);
    int SetNCOFreq(bool tx,size_t ch, const float_type *freq, float_type pho);
    int SetNCO(bool tx,size_t ch,size_t ind,bool down);
    int GetNCOFreq(bool tx,size_t ch, float_type * freq,float_type *pho);
    int SetNCOPhase(bool tx,size_t ch, const float_type *phase, float_type fcw);
    int GetNCOPhase(bool tx,size_t ch, float_type * phase,float_type *fcw);
    size_t GetNCO(bool tx,size_t ch);
    int Calibrate(bool dir_tx, size_t chan, double bw, unsigned flags);
    int CalibrateInternalADC();
    int CalibrateAnalogRSSI_DC_Offset();
    int CalibrateRP_BIAS();
    int CalibrateTxGain(float maxGainOffset_dBFS, float *actualGain_dBFS);
    int ProgramFPGA(const char* data, size_t len, lms_target_t mode,lime::IConnection::ProgrammingCallback callback);
    int ProgramFPGA(std::string name, lms_target_t mode,lime::IConnection::ProgrammingCallback callback);
    int ProgramFW(const char* data, size_t len, lms_target_t mode,lime::IConnection::ProgrammingCallback callback);
    int ProgramFW(std::string name, lms_target_t mode,lime::IConnection::ProgrammingCallback callback);
    int ProgramHPM7(const char* data, size_t len, int mode,lime::IConnection::ProgrammingCallback callback);
    int ProgramHPM7(std::string fname, int mode,lime::IConnection::ProgrammingCallback callback);
    int ProgramUpdate(const bool download, lime::IConnection::ProgrammingCallback callback);
    int ProgramMCU(const char* data=nullptr, size_t len=0, lms_target_t mode=LMS_TARGET_RAM,lime::IConnection::ProgrammingCallback callback=nullptr);
    int DACWrite(uint16_t val);
    int DACRead();
    int GetClockFreq(size_t clk_id, float_type *freq);
    int SetClockFreq(size_t clk_id, float_type freq);
    lms_dev_info_t* GetInfo();
    int Synchronize(bool toChip);
    int SetLogCallback(void(*func)(const char* cstr, const unsigned int type));
    int EnableCalibCache(bool enable);
    int GetChipTemperature(size_t ind, float_type *temp);
    int LoadConfig(const char *filename);
    int SaveConfig(const char *filename);
    int ReadLMSReg(uint16_t address, uint16_t *val);
    int WriteLMSReg(uint16_t address, uint16_t val);
    int RegisterTest();
    int ReadParam(struct LMS7Parameter param, uint16_t *val);
    int WriteParam(struct LMS7Parameter param, uint16_t val);
    int SetActiveChip(unsigned ind);
    lime::LMS7002M* GetLMS(int index = -1);
    static LMS7_Device* CreateDevice(lime::IConnection* conn, LMS7_Device *obj = nullptr);
    std::map<std::string, double> extra_parameters;
protected:
    lms_dev_info_t devInfo;
    std::vector<lms_channel_info> tx_channels;
    std::vector<lms_channel_info> rx_channels;
    static const double LMS_CGEN_MAX;
    lime::IConnection* connection;
    std::vector<lime::LMS7002M*> lms_list;
    int ConfigureRXLPF(bool enabled,int ch,float_type bandwidth);
    int ConfigureTXLPF(bool enabled,int ch,float_type bandwidth);
    int ConfigureGFIR(bool enabled,bool tx, float_type bandwidth,size_t ch);
    void _Initialize(lime::IConnection* conn);
    unsigned lms_chip_id;
};

#endif	/* LMS7_DEVICE_H */

