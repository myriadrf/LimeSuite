#include "LimeSDR_Mini.h"

#include "USBGeneric.h"
#include "LMSBoards.h"
#include "limesuite/LMS7002M.h"
#include "Si5351C/Si5351C.h"
#include "LMS64CProtocol.h"
#include "Logger.h"
#include "FPGA_common.h"
#include "TRXLooper_USB.h"
#include "limesuite/LMS7002M_parameters.h"
#include "protocols/LMS64CProtocol.h"
#include "limesuite/DeviceNode.h"
#include "ADCUnits.h"

#include <assert.h>
#include <memory>
#include <set>
#include <stdexcept>
#include <cmath>

#ifdef __unix__
    #include <libusb.h>
#endif

using namespace lime;

LimeSDR_Mini::LimeSDR_Mini(lime::IComms* spiLMS, lime::IComms* spiFPGA, USBGeneric* streamPort)
    : mStreamPort(streamPort),
    mlms7002mPort(spiLMS),
    mfpgaPort(spiFPGA)
{
}

LimeSDR_Mini::~LimeSDR_Mini()
{
}

// Verify and configure given settings
// throw logic_error with description why the config is not possible
inline bool InRange(double val, double min, double max)
{
    return val >= min ? val <= max : false;
}

static inline const std::string strFormat(const char *format, ...)
{
    char ctemp[256];

    va_list args;
    va_start(args, format);
    vsnprintf(ctemp, 256, format, args);
    va_end(args);

    return std::string(ctemp);
}

void LimeSDR_Mini::Configure(const SDRConfig& cfg, uint8_t moduleIndex = 0)
{
}

void LimeSDR_Mini::SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase, double rxPhase)
{
}

int LimeSDR_Mini::Init()
{
}

void LimeSDR_Mini::Reset()
{
    LMS64CProtocol::DeviceReset(reinterpret_cast<ISerialPort&>(mStreamPort), 0);
}

double LimeSDR_Mini::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
}

void LimeSDR_Mini::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
}

void LimeSDR_Mini::Synchronize(bool toChip)
{
}

void LimeSDR_Mini::EnableCache(bool enable)
{
}

void LimeSDR_Mini::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
}

int LimeSDR_Mini::StreamSetup(const StreamConfig &config, uint8_t moduleIndex)
{
}

void LimeSDR_Mini::StreamStart(uint8_t moduleIndex)
{
}

void LimeSDR_Mini::StreamStop(uint8_t moduleIndex)
{
}

void LimeSDR_Mini::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx)
{
}

void *LimeSDR_Mini::GetInternalChip(uint32_t index)
{
}

int LimeSDR_Mini::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
}

int LimeSDR_Mini::GPIORead(uint8_t *buffer, const size_t bufLength)
{
}

int LimeSDR_Mini::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
}

int LimeSDR_Mini::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
}

int LimeSDR_Mini::CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units)
{
}

int LimeSDR_Mini::CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units)
{

}

int LimeSDR_Mini::ReadFPGARegister(uint32_t address)
{
}

int LimeSDR_Mini::WriteFPGARegister(uint32_t address, uint32_t value)
{
}
