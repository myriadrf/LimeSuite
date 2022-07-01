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
#include <stdexcept>

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

static const uint8_t spi_LMS7002M = 0;
static const uint8_t spi_FPGA = 1;

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
    assert(comms);
    DeviceInfo devInfo;
    try
    {
        LMS64CProtocol::LMS64CPacket pkt;
        pkt.cmd = CMD_GET_INFO;
        int sentBytes = comms->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR, CTR_W_REQCODE, CTR_W_VALUE, CTR_W_INDEX, (uint8_t*)&pkt, sizeof(pkt), 1000);
        if (sentBytes != sizeof(pkt))
            throw std::runtime_error("LimeSDR::GetDeviceInfo write failed");
        int gotBytes = comms->ControlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, CTR_R_REQCODE, CTR_R_VALUE, CTR_R_INDEX, (uint8_t*)&pkt, sizeof(pkt), 1000);
        if (gotBytes != sizeof(pkt))
            throw std::runtime_error("LimeSDR::GetDeviceInfo read failed");

        LMS64CProtocol::LMSinfo info;
        if (pkt.status == STATUS_COMPLETED_CMD && gotBytes >= pkt.headerSize)
        {
            info.firmware = pkt.payload[0];
            info.device = pkt.payload[1] < LMS_DEV_COUNT ? (eLMS_DEV)pkt.payload[1] : LMS_DEV_UNKNOWN;
            info.protocol = pkt.payload[2];
            info.hardware = pkt.payload[3];
            info.expansion = pkt.payload[4] < EXP_BOARD_COUNT ? (eEXP_BOARD)pkt.payload[4] : EXP_BOARD_UNKNOWN;
            info.boardSerialNumber = 0;
            for (int i = 10; i < 18; i++)
            {
                info.boardSerialNumber <<= 8;
                info.boardSerialNumber |= pkt.payload[i];
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

        LMS64CProtocol::FPGAinfo gatewareInfo;
        const uint32_t addrs[] = {0x0000, 0x0001, 0x0002, 0x0003};
        uint32_t data[4];
        SPI(spi_FPGA, addrs, data, 4);
        gatewareInfo.boardID = (eLMS_DEV)data[0];//(pkt.inBuffer[2] << 8) | pkt.inBuffer[3];
        gatewareInfo.gatewareVersion = data[1];//(pkt.inBuffer[6] << 8) | pkt.inBuffer[7];
        gatewareInfo.gatewareRevision = data[2];//(pkt.inBuffer[10] << 8) | pkt.inBuffer[11];
        gatewareInfo.hwVersion = data[3] & 0x7F;//pkt.inBuffer[15]&0x7F;

        devInfo.gatewareTargetBoard = GetDeviceName(eLMS_DEV(gatewareInfo.boardID));
        devInfo.gatewareVersion = std::to_string(int(gatewareInfo.gatewareVersion));
        devInfo.gatewareRevision = std::to_string(int(gatewareInfo.gatewareRevision));
        devInfo.hardwareVersion = std::to_string(int(gatewareInfo.hwVersion));
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
    LMS64CProtocol::LMS64CPacket pkt;
    for(int i=0; i<count; ++i)
    {
        pkt.blockCount = 1;
        pkt.periphID = chipSelect;
        if(MOSI[i] & (1<<31)) // SPI write bit
        {
            switch(chipSelect)
            {
                case spi_LMS7002M: pkt.cmd = CMD_LMS7002_WR; break;
                case spi_FPGA: pkt.cmd = CMD_BRDSPI_WR; break;
                default: throw std::logic_error("LimeSDR SPI invalid SPI chip select");
            }
            pkt.payload[0] = MOSI[i] >> 24;
            pkt.payload[9] = MOSI[i] >> 16;
            pkt.payload[10] = MOSI[i] >> 8;
            pkt.payload[11] = MOSI[i];
        }
        else
        {
            switch(chipSelect)
            {
                case spi_LMS7002M: pkt.cmd = CMD_LMS7002_RD; break;
                case spi_FPGA: pkt.cmd = CMD_BRDSPI_RD; break;
                default: throw std::logic_error("LimeSDR SPI invalid SPI chip select");
            }
            pkt.payload[0] = MOSI[i] >> 8;
            pkt.payload[1] = MOSI[i];
        }
        int sent = comms->BulkTransfer(ctrlBulkOutAddr, (uint8_t*)&pkt, sizeof(pkt), 100);
        if (sent != sizeof(pkt))
            throw std::runtime_error("SPI failed");

        int recv = comms->BulkTransfer(ctrlBulkInAddr, (uint8_t*)&pkt, sizeof(pkt), 100);

        if (recv >= pkt.headerSize+4 && pkt.status == STATUS_COMPLETED_CMD)
        {
            MISO[i] = pkt.payload[0] << 24;
            MISO[i] |= pkt.payload[1] << 16;
            MISO[i] |= pkt.payload[2] << 8;
            MISO[i] |= pkt.payload[3];
        }
        else
            throw std::runtime_error("SPI failed");
    }
}
