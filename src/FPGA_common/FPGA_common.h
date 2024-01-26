/**
@file FPGA_common.h
@author Lime Microsystems
@brief Common functions used to work with FPGA
*/

#ifndef FPGA_COMMON_H
#define FPGA_COMMON_H
#include <stdint.h>
#include <map>
#include <memory>

#include "SamplesPacket.h"
#include "DataPacket.h"
#include "TRXLooper.h"
#include "limesuite/complex.h"

namespace lime {
class ISPI;

/** @brief Class for interfacing with a field-programmable gate array (FPGA). */
class FPGA
{
  public:
    FPGA(std::shared_ptr<ISPI> fpgaSPI, std::shared_ptr<ISPI> lms7002mSPI);
    virtual ~FPGA(){};

    int StartStreaming();
    int StopStreaming();
    int ResetTimestamp();

    /** @brief Structure for holding FPGA's Phase-Locked Loop (PLL) clock information. */
    struct FPGA_PLL_clock {
        FPGA_PLL_clock()
            : outFrequency(0)
            , phaseShift_deg(0)
            , index(0)
            , bypass(false)
            , findPhase(false)
            , rd_actualFrequency(0)
        {
        }
        double outFrequency;
        double phaseShift_deg;
        uint8_t index;
        bool bypass;
        bool findPhase;
        double rd_actualFrequency;
    };

    virtual int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, double txPhase, double rxPhase, int ch = 0);
    virtual int SetInterfaceFreq(double f_Tx_Hz, double f_Rx_Hz, int ch = 0);
    double DetectRefClk(double fx3Clk = 100e6);

    static int FPGAPacketPayload2Samples(const std::byte* buffer, int bufLen, bool mimo, bool compressed, complex16_t** samples);
    static int FPGAPacketPayload2SamplesFloat(
        const std::byte* buffer, int bufLen, bool mimo, bool compressed, complex32f_t** samples);
    static int Samples2FPGAPacketPayload(
        const complex16_t* const* samples, int samplesCount, bool mimo, bool compressed, std::byte* buffer);
    static int Samples2FPGAPacketPayloadFloat(
        const complex32f_t* const* samples, int samplesCount, bool mimo, bool compressed, std::byte* buffer);
    virtual void EnableValuesCache(bool enabled);
    virtual int WriteRegisters(const uint32_t* addrs, const uint32_t* data, unsigned cnt);
    virtual int ReadRegisters(const uint32_t* addrs, uint32_t* data, unsigned cnt);
    int WriteRegister(uint32_t addr, uint32_t val);
    int ReadRegister(uint32_t addr);
    int WriteLMS7002MSPI(const uint32_t* addr, uint32_t length);
    int ReadLMS7002MSPI(const uint32_t* addr, uint32_t* values, uint32_t length);

    /** @brief Structure containing the gateware information of the FPGA */
    struct GatewareInfo {
        int boardID;
        int version;
        int revision;
        int hardwareVersion;
    };
    GatewareInfo GetGatewareInfo();
    static void GatewareToDescriptor(const FPGA::GatewareInfo& gw, SDRDevice::Descriptor& desc);

  protected:
    int WaitTillDone(uint16_t pollAddr, uint16_t doneMask, uint16_t errorMask, const std::string& title = "");
    int SetPllFrequency(uint8_t pllIndex, double inputFreq, FPGA_PLL_clock* outputs, uint8_t clockCount);
    int SetDirectClocking(int clockIndex);
    std::shared_ptr<ISPI> fpgaPort;
    std::shared_ptr<ISPI> lms7002mPort;

  private:
    virtual int ReadRawStreamData(char* buffer, unsigned length, int epIndex, int timeout_ms);
    int SetPllClock(uint8_t clockIndex, int nSteps, bool waitLock, bool doPhaseSearch, uint16_t& reg23val);
    bool useCache;
    std::map<uint16_t, uint16_t> regsCache;
};

} // namespace lime
#endif // FPGA_COMMON_H
