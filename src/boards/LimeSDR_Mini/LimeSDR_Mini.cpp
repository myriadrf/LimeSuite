#include "LimeSDR_Mini.h"

#include "USBGeneric.h"
#include "LMSBoards.h"
#include "limesuite/LMS7002M.h"
#include "Si5351C/Si5351C.h"
#include "LMS64CProtocol.h"
#include "Logger.h"
#include "FPGA_Mini.h"
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
    #include "libusb.h"
#endif

using namespace lime;

static constexpr uint8_t spi_LMS7002M = 0;
static constexpr uint8_t spi_FPGA = 1;

static const SDRDevice::CustomParameter CP_VCTCXO_DAC = {"VCTCXO DAC (runtime)", 0, 0, 255, false};

LimeSDR_Mini::LimeSDR_Mini(lime::IComms* spiLMS, lime::IComms* spiFPGA, USBGeneric* streamPort)
    : mStreamPort(streamPort),
    mlms7002mPort(spiLMS),
    mfpgaPort(spiFPGA)
{
    SDRDevice::Descriptor descriptor = mDeviceDescriptor;
    descriptor.name = GetDeviceName(LMS_DEV_LIMESDRMINI);

    mLMSChips.push_back(new LMS7002M(mlms7002mPort));
    mLMSChips[0]->SetConnection(mlms7002mPort);

    mFPGA = new FPGA_Mini(spiFPGA, spiLMS);
    FPGA::GatewareInfo gw = mFPGA->GetGatewareInfo();
    FPGA::GatewareToDescriptor(gw, descriptor);

    mStreamers.resize(1, nullptr);

    descriptor.customParameters.push_back(CP_VCTCXO_DAC);

    descriptor.spiSlaveIds = {{"LMS7002M", spi_LMS7002M}, {"FPGA", spi_FPGA}};

    RFSOCDescriptor soc;
    soc.channelCount = 2;
    soc.rxPathNames = {"NONE", "LNAH", "LNAL_NC", "LNAW", "Auto"};
    soc.txPathNames = {"NONE", "BAND1", "BAND2", "Auto"};

    descriptor.rfSOC.push_back(soc);

    DeviceNode* fpgaNode = new DeviceNode("FPGA", "FPGA-Mini", mFPGA);
    fpgaNode->childs.push_back(new DeviceNode("LMS", "LMS7002M", mLMSChips[0]));
    descriptor.socTree = new DeviceNode("SDR Mini", "SDRDevice", this);
    descriptor.socTree->childs.push_back(fpgaNode);

    mDeviceDescriptor = descriptor;
}

LimeSDR_Mini::~LimeSDR_Mini()
{
    lime::error("LimeSDR_Mini::~LimeSDR_Mini stub");
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
    lime::error("LimeSDR_Mini::Configure stub");
}

void LimeSDR_Mini::SetFPGAInterfaceFreq(uint8_t interp, uint8_t dec, double txPhase, double rxPhase)
{
    lime::error("LimeSDR_Mini::SetFPGAInterfaceFreq stub");
}

int LimeSDR_Mini::Init()
{
    struct regVal
    {
        uint16_t adr;
        uint16_t val;
    };

    const std::vector<regVal> initVals_1v0 = {
        {0x0022, 0x0FFF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x03F0},
        {0x0089, 0x1078}, {0x008B, 0x2100}, {0x008C, 0x267B}, {0x0092, 0xFFFF},
	    {0x0093, 0x03FF}, {0x00A1, 0x656A}, {0x00A6, 0x0001}, {0x00A9, 0x8000},
        {0x00AC, 0x2000}, {0x0105, 0x0011}, {0x0108, 0x218C}, {0x0109, 0x6100},
        {0x010A, 0x1F4C}, {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010E, 0x0000},
        {0x010F, 0x3142}, {0x0110, 0x2B14}, {0x0111, 0x0000}, {0x0112, 0x942E},
        {0x0113, 0x03C2}, {0x0114, 0x00D0}, {0x0117, 0x1230}, {0x0119, 0x18D2},
        {0x011C, 0x8941}, {0x011D, 0x0000}, {0x011E, 0x0740}, {0x0120, 0xE6C0},
        {0x0121, 0x8650}, {0x0123, 0x000F}, {0x0200, 0x00E1}, {0x0208, 0x017B},
        {0x020B, 0x4000}, {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006},
        {0x040B, 0x1020}, {0x040C, 0x00FB}
    };

    const std::vector<regVal> initVals_1v2 = {
        {0x0022, 0x0FFF}, {0x0023, 0x5550}, {0x002B, 0x0038}, {0x002C, 0x0000},
        {0x002D, 0x0641}, {0x0086, 0x4101}, {0x0087, 0x5555}, {0x0088, 0x03F0},
        {0x0089, 0x1078}, {0x008B, 0x2100}, {0x008C, 0x267B}, {0x00A1, 0x656A},
        {0x00A6, 0x0009}, {0x00A7, 0x8A8A}, {0x00A9, 0x8000}, {0x00AC, 0x2000},
        {0x0105, 0x0011}, {0x0108, 0x218C}, {0x0109, 0x6100}, {0x010A, 0x1F4C},
        {0x010B, 0x0001}, {0x010C, 0x8865}, {0x010E, 0x0000}, {0x010F, 0x3142},
        {0x0110, 0x2B14}, {0x0111, 0x0000}, {0x0112, 0x942E}, {0x0113, 0x03C2},
        {0x0114, 0x00D0}, {0x0117, 0x1230}, {0x0119, 0x18D2}, {0x011C, 0x8941},
        {0x011D, 0x0000}, {0x011E, 0x0740}, {0x0120, 0xC5C0}, {0x0121, 0x8650},
        {0x0123, 0x000F}, {0x0200, 0x00E1}, {0x0208, 0x017B}, {0x020B, 0x4000},
        {0x020C, 0x8000}, {0x0400, 0x8081}, {0x0404, 0x0006}, {0x040B, 0x1020},
        {0x040C, 0x00FB}
    };

    int hw_version = mFPGA->ReadRegister(3) & 0xF;
    auto &initVals = hw_version >= 2 ? initVals_1v2 : initVals_1v0;

    lime::LMS7002M* lms = mLMSChips[0];

    if (lms->ResetChip() != 0)
    {
        return -1;
    }

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);
    for (auto i : initVals)
    {
        lms->SPI_write(i.adr, i.val, true);
    }

    if (lms->CalibrateTxGain(0, nullptr) != 0)
    {
        return -1;
    }

    lms->EnableChannel(TRXDir::Tx, 0, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 2);
    lms->SPI_write(0x0123, 0x000F);  //SXT
    lms->SPI_write(0x0120, 0x80C0);  //SXT
    lms->SPI_write(0x011C, 0x8941);  //SXT
    lms->EnableChannel(TRXDir::Rx, 0, false);
    lms->EnableChannel(TRXDir::Tx, 0, false);

    lms->Modify_SPI_Reg_bits(LMS7param(MAC), 1);

    // bool auto_path[2] = {auto_tx_path, auto_rx_path};
    // auto_tx_path = false;
    // auto_rx_path = false;
    
    // if(SetFrequency(true, 0, GetFrequency(true, 0)) != 0)
    // {
    //     return -1;
    // }

    // if(SetFrequency(false, 0, GetFrequency(false, 0)) != 0)
    // {
    //     return -1;
    // }

    // auto_tx_path = auto_path[0];
    // auto_rx_path = auto_path[1];

    // if (SetRate(15.36e6, 1) != 0)
    // {
    //     return -1;
    // }

    return 0;
}

void LimeSDR_Mini::Reset()
{
    LMS64CProtocol::DeviceReset(reinterpret_cast<ISerialPort&>(mStreamPort), 0);
}

double LimeSDR_Mini::GetClockFreq(uint8_t clk_id, uint8_t channel)
{
    lime::error("LimeSDR_Mini::GetClockFreq stub");
}

void LimeSDR_Mini::SetClockFreq(uint8_t clk_id, double freq, uint8_t channel)
{
    lime::error("LimeSDR_Mini::SetClockFreq stub");
}

void LimeSDR_Mini::Synchronize(bool toChip)
{
    lime::error("LimeSDR_Mini::Synchronize stub");
}

void LimeSDR_Mini::EnableCache(bool enable)
{
    lime::error("LimeSDR_Mini::EnableCache stub");
}

void LimeSDR_Mini::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, uint32_t count)
{
    lime::error("LimeSDR_Mini::SPI stub");
}

int LimeSDR_Mini::StreamSetup(const StreamConfig &config, uint8_t moduleIndex)
{
    lime::error("LimeSDR_Mini::StreamSetup stub");
}

void LimeSDR_Mini::StreamStart(uint8_t moduleIndex)
{
    lime::error("LimeSDR_Mini::StreamStart stub");
}

void LimeSDR_Mini::StreamStop(uint8_t moduleIndex)
{
    lime::error("LimeSDR_Mini::StreamStop stub");
}

void LimeSDR_Mini::StreamStatus(uint8_t moduleIndex, SDRDevice::StreamStats* rx, SDRDevice::StreamStats* tx)
{
    lime::error("LimeSDR_Mini::StreamStatus stub");
}

void *LimeSDR_Mini::GetInternalChip(uint32_t index)
{
    lime::error("LimeSDR_Mini::GetInternalChip stub");
}

int LimeSDR_Mini::GPIODirRead(uint8_t *buffer, const size_t bufLength)
{
    lime::error("LimeSDR_Mini::GPIODirRead stub");
}

int LimeSDR_Mini::GPIORead(uint8_t *buffer, const size_t bufLength)
{
    lime::error("LimeSDR_Mini::GPIORead stub");
}

int LimeSDR_Mini::GPIODirWrite(const uint8_t *buffer, const size_t bufLength)
{
    lime::error("LimeSDR_Mini::GPIODirWrite stub");
}

int LimeSDR_Mini::GPIOWrite(const uint8_t *buffer, const size_t bufLength)
{
    lime::error("LimeSDR_Mini::GPIOWrite stub");
}

int LimeSDR_Mini::CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units)
{
    lime::error("LimeSDR_Mini::CustomParameterWrite stub");
}

int LimeSDR_Mini::CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units)
{
    lime::error("LimeSDR_Mini::CustomParameterRead stub");
}

int LimeSDR_Mini::ReadFPGARegister(uint32_t address)
{
    lime::error("LimeSDR_Mini::ReadFPGARegister stub");
}

int LimeSDR_Mini::WriteFPGARegister(uint32_t address, uint32_t value)
{
    lime::error("LimeSDR_Mini::WriteFPGARegister stub");
}
