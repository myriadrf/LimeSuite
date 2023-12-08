/**
    @file ConnectionImages.cpp
    @author Lime Microsystems
    @brief Image updating and version checking
*/

#include "SystemResources.h"
#include "LMS64CProtocol.h"
#include "LMSBoards.h"
#include "Logger.h"
#include <fstream>
#include <ciso646>

using namespace lime;

/** @brief The entry structure that describes a board revision and its firmware and gateware images */
struct ConnectionImageEntry {
    eLMS_DEV dev; ///< The identifier of the device
    int hw_rev; ///< The hardware revision of the device

    int fw_ver; ///< The firmware version of the device
    const char* fw_img; ///< The firmware image of the device

    int gw_ver; ///< The gateware version of the device
    int gw_rev; ///< The gateware revision of the device
    const char* gw_img; ///< The gateware image of the device
};

/*!
 * Lookup the board information given hardware type and revision.
 * Edit each entry for supported hardware and image updates.
 */
static const ConnectionImageEntry& lookupImageEntry(const LMS64CProtocol::LMSinfo& info)
{
    static const std::vector<ConnectionImageEntry> imageEntries = {
        ConnectionImageEntry({ LMS_DEV_UNKNOWN, -1, -1, nullptr, -1, -1, nullptr }),
        ConnectionImageEntry({ LMS_DEV_LIMESDR, 4, 4, "LimeSDR-USB_HW_1.4_r4.0.img", 2, 23, "LimeSDR-USB_HW_1.4_r2.23.rbf" }),
        ConnectionImageEntry({ LMS_DEV_LIMESDR, 3, 3, "LimeSDR-USB_HW_1.3_r3.0.img", 1, 20, "LimeSDR-USB_HW_1.1_r1.20.rbf" }),
        ConnectionImageEntry({ LMS_DEV_LIMESDR, 2, 3, "LimeSDR-USB_HW_1.2_r3.0.img", 1, 20, "LimeSDR-USB_HW_1.1_r1.20.rbf" }),
        ConnectionImageEntry({ LMS_DEV_LIMESDR, 1, 7, "LimeSDR-USB_HW_1.1_r7.0.img", 1, 20, "LimeSDR-USB_HW_1.1_r1.20.rbf" }),
        ConnectionImageEntry({ LMS_DEV_STREAM, 3, 8, "STREAM-USB_HW_1.1_r8.0.img", 1, 2, "STREAM-USB_HW_1.3_r1.2.rbf" }),
        ConnectionImageEntry({ LMS_DEV_LIMENET_MICRO, 3, 0, nullptr, 1, 3, "LimeNET-Micro_lms7_trx_HW_2.1_r1.3.rpd" }),
        ConnectionImageEntry({ LMS_DEV_LIMESDRMINI, 0, 0, nullptr, 1, 30, "LimeSDR-Mini_HW_1.2_r1.30.rpd" }),
        ConnectionImageEntry({ LMS_DEV_LIMESDRMINI_V2, 0, 0, nullptr, 2, 2, "LimeSDR-Mini_HW_2.0_r2.2.bit" }),
    };

    for (const auto& iter : imageEntries)
    {
        if (info.device == iter.dev && info.hardware == iter.hw_rev)
        {
            return iter;
        }
    }

    return imageEntries.front(); //the -1 unknown entry
}

static bool programmingCallbackStream(
    int bsent, int btotal, const char* progressMsg, const std::string& image, IConnection::ProgrammingCallback callback)
{
    const auto msg = std::string(progressMsg) + " (" + image + ")";
    return callback(bsent, btotal, msg.c_str());
}

int LMS64CProtocol::ProgramUpdate(const bool download, const bool force, IConnection::ProgrammingCallback callback)
{
    const auto info = this->GetInfo();
    const auto& entry = lookupImageEntry(info);
    bool fwUpdateNeeded = true;
    bool gwUpdateNeeded = true;

    //an entry match was not found
    if (entry.dev == LMS_DEV_UNKNOWN)
    {
        return lime::ReportError("Update not supported: %s[HW=%d]", GetDeviceName(info.device), info.hardware);
    }

    //download images when missing
    if (download)
    {
        std::vector<std::string> images = { entry.gw_img };
        if (entry.fw_img)
            images.push_back(std::string(entry.fw_img));
        for (const auto& image : images)
        {
            if (not lime::locateImageResource(image).empty())
                continue;
            const std::string msg("Downloading: " + image);
            if (callback)
                callback(0, 1, msg.c_str());
            int ret = lime::downloadImageResource(image);
            if (ret != 0)
                return ret; //error set by download call
            if (callback)
                callback(1, 1, "Done!");
        }
    }
    if (!force)
    {
        const auto fpgaInfo = this->GetFPGAInfo();
        if (entry.fw_img && info.firmware == entry.fw_ver)
        {
            lime::info("Existing firmware is same as update (%d)", info.firmware);
            fwUpdateNeeded = false;
        }
        if (entry.gw_img && fpgaInfo.gatewareVersion == entry.gw_ver && fpgaInfo.gatewareRevision == entry.gw_rev)
        {
            lime::info("Existing gateware is same as update (%d.%d)", fpgaInfo.gatewareVersion, fpgaInfo.gatewareRevision);
            gwUpdateNeeded = false;
        }
        if (!(gwUpdateNeeded || fwUpdateNeeded))
        {
            lime::info("Firmware and Gateware update is not required.");
            return 0;
        }
    }
    //load firmware into flash
    if (fwUpdateNeeded && entry.fw_img)
    {
        //open file
        std::ifstream file;
        const auto path = lime::locateImageResource(entry.fw_img);
        file.open(path.c_str(), std::ios::in | std::ios::binary);
        if (not file.good())
            return lime::ReportError("Error opening %s", path.c_str());

        //read file
        std::streampos fileSize;
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> progData(fileSize, 0);
        file.read(progData.data(), fileSize);

        int device = LMS64CProtocol::FX3; //FX3
        int progMode = 2; //Firmware to FLASH
        using namespace std::placeholders;
        const auto cb = std::bind(&programmingCallbackStream, _1, _2, _3, path, callback);
        auto status = this->ProgramWrite(progData.data(), progData.size(), progMode, device, cb);
        if (status != 0)
            return status;
    }

    //load gateware into flash
    if (gwUpdateNeeded && entry.gw_img)
    {
        //open file
        std::ifstream file;
        const auto path = lime::locateImageResource(entry.gw_img);
        file.open(path, std::ios::in | std::ios::binary);
        if (not file.good())
            return lime::ReportError("Error opening %s", path.c_str());

        //read file
        std::streampos fileSize;
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> progData(fileSize, 0);
        file.read(progData.data(), fileSize);

        int device = LMS64CProtocol::FPGA; //Altera FPGA
        int progMode = 1; //Bitstream to FLASH
        using namespace std::placeholders;
        const auto cb = std::bind(&programmingCallbackStream, _1, _2, _3, path, callback);
        auto status = this->ProgramWrite(progData.data(), progData.size(), progMode, device, cb);
        if (status != 0)
            return status;
    }

    //Reset FX3, FPGA should be reloaded on boot
    if (entry.fw_img)
    {
        int device = LMS64CProtocol::FX3; //FX3
        auto status = this->ProgramWrite(nullptr, 0, 0, device, nullptr);
        if (status != 0)
            return status;
    }
    return 0;
}
