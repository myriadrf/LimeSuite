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

/*!
 * The entry structure that describes a board revision and its fw/gw images
 */
struct ConnectionImageEntry {
    eLMS_DEV dev;
    int hw_rev;

    int fw_ver;
    const char* fw_img;

    int gw_ver;
    int gw_rev;
    const char* gw_img;
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
