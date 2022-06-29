#include "LimeSDR.h"

#include "USBGeneric.h"
#include "LMSBoards.h"
#include "LMS7002M.h"
#include "Si5351C.h"
#include "LMS64CProtocol.h"
#include "Logger.h"

#include <assert.h>
#include <memory>
#include <set>

#ifdef __unix__
    #include "libusb.h"
#endif

using namespace lime;

#define CTR_W_REQCODE 0xC1
#define CTR_W_VALUE 0x0000
#define CTR_W_INDEX 0x0000

#define CTR_R_REQCODE 0xC0
#define CTR_R_VALUE 0x0000
#define CTR_R_INDEX 0x0000

using namespace lime;

static const uint8_t ctrlBulkOutAddr = 0x0F;
static const uint8_t ctrlBulkInAddr = 0x8F;

//control commands to be send via bulk port for boards v1.2 and later
static const std::set<uint8_t> commandsToBulkCtrlHw2 =
{
    CMD_BRDSPI_WR, CMD_BRDSPI_RD,
    CMD_LMS7002_WR, CMD_LMS7002_RD,
    CMD_ANALOG_VAL_WR, CMD_ANALOG_VAL_RD,
    CMD_ADF4002_WR,
    CMD_LMS7002_RST,
    CMD_GPIO_DIR_WR, CMD_GPIO_DIR_RD,
    CMD_GPIO_WR, CMD_GPIO_RD,
};


LimeSDR::LimeSDR(lime::USBGeneric* conn) : comms(conn)
{
    const uint8_t spiSlaveId = 0;
    mLMSChip = new LMS7002M(spiSlaveId);

    //must configure synthesizer before using LimeSDR
    //if (info.device == LMS_DEV_LIMESDR && info.hardware < 4)
    /*{
        std::shared_ptr<Si5351C> si5351module(new Si5351C());
        si5351module->Initialize(conn);
        si5351module->SetPLL(0, 25000000, 0);
        si5351module->SetPLL(1, 25000000, 0);
        si5351module->SetClock(0, 27000000, true, false);
        si5351module->SetClock(1, 27000000, true, false);
        for (int i = 2; i < 8; ++i)
            si5351module->SetClock(i, 27000000, false, false);
        Si5351C::Status status = si5351module->ConfigureClocks();
        if (status != Si5351C::SUCCESS)
        {
            lime::warning("Failed to configure Si5351C");
            return;
        }
        status = si5351module->UploadConfiguration();
        if (status != Si5351C::SUCCESS)
            lime::warning("Failed to upload Si5351C configuration");
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); //some settle time
    }*/
}

LimeSDR::~LimeSDR()
{
    delete mLMSChip;
    delete comms;
}

DeviceInfo LimeSDR::GetDeviceInfo(void)
{
    DeviceInfo devInfo;
    try
    {
        assert(comms);
        uint8_t data[64];
        memset(data, 0, sizeof(data));
        data[0] = CMD_GET_INFO;
        int sentBytes = comms->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR, CTR_W_REQCODE, CTR_W_VALUE, CTR_W_INDEX, data, sizeof(data), 1000);
        int gotBytes = comms->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, CTR_R_REQCODE, CTR_R_VALUE, CTR_R_INDEX, data, sizeof(data), 1000);

        LMS64CProtocol::LMSinfo info;
        uint8_t* payload = data+8;
        if (data[1] == 1 && gotBytes >= 5)
        {
            info.firmware = payload[0];
            info.device = payload[1] < LMS_DEV_COUNT ? (eLMS_DEV)payload[1] : LMS_DEV_UNKNOWN;
            info.protocol = payload[2];
            info.hardware = payload[3];
            info.expansion = payload[4] < EXP_BOARD_COUNT ? (eEXP_BOARD)payload[4] : EXP_BOARD_UNKNOWN;
            info.boardSerialNumber = 0;
            for (int i = 10; i < 18; i++)
            {
                info.boardSerialNumber <<= 8;
                info.boardSerialNumber |= payload[i];
            }
        }
        else
            return devInfo;
        devInfo.deviceName = GetDeviceName(info.device);
        devInfo.expansionName = GetExpansionBoardName(info.expansion);
        devInfo.firmwareVersion = std::to_string(int(info.firmware));
        devInfo.hardwareVersion = std::to_string(int(info.hardware));
        devInfo.protocolVersion = std::to_string(int(info.protocol));
        devInfo.boardSerialNumber = info.boardSerialNumber;

        // FPGAinfo gatewareInfo = this->GetFPGAInfo();
        // devInfo.gatewareTargetBoard = GetDeviceName(eLMS_DEV(gatewareInfo.boardID));
        // devInfo.gatewareVersion = std::to_string(int(gatewareInfo.gatewareVersion));
        // devInfo.gatewareRevision = std::to_string(int(gatewareInfo.gatewareRevision));
        // devInfo.hardwareVersion = std::to_string(int(gatewareInfo.hwVersion));
        return devInfo;
    }
    catch (...)
    {
        //lime::error("LimeSDR::GetDeviceInfo failed(%s)", e.what());
        devInfo.deviceName = GetDeviceName(LMS_DEV_UNKNOWN);
        devInfo.expansionName = GetExpansionBoardName(EXP_BOARD_UNKNOWN);
    }
    return devInfo;
}

void LimeSDR::SPI(uint32_t chipSelect, const uint32_t *MOSI, uint32_t *MISO, size_t count)
{
    assert(comms);
    try
    {

    }
    catch(std::exception& e)
    {

    }
}
