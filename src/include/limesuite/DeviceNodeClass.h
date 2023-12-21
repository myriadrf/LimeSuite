#ifndef LIME_DEVICENODECLASS_H
#define LIME_DEVICENODECLASS_H

#include <cstdint>

namespace lime {

enum class eDeviceNodeClass : uint8_t {
    ADF4002,
    CDCM6208,
    FPGA,
    FPGA_MINI,
    FPGA_X3,
    FPGA_XTRX,
    LMS7002M,
    SDRDevice,
};

}

#endif // LIME_DEVICENODECLASS_H