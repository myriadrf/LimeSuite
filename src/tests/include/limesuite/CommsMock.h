#ifndef LIME_COMMSMOCK_H
#define LIME_COMMSMOCK_H

#include <gmock/gmock.h>

#include "limesuite/IComms.h"

namespace lime::testing {

class CommsMock : public IComms
{
  public:
    MOCK_METHOD(OpStatus, SPI, (const uint32_t* MOSI, uint32_t* MISO, uint32_t count), (override));
    MOCK_METHOD(OpStatus, SPI, (uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count), (override));
    MOCK_METHOD(OpStatus, GPIODirRead, (uint8_t * buffer, const size_t bufLength), (override));
    MOCK_METHOD(OpStatus, GPIORead, (uint8_t * buffer, const size_t bufLength), (override));
    MOCK_METHOD(OpStatus, GPIODirWrite, (const uint8_t* buffer, const size_t bufLength), (override));
    MOCK_METHOD(OpStatus, GPIOWrite, (const uint8_t* buffer, const size_t bufLength), (override));
    MOCK_METHOD(OpStatus, CustomParameterWrite, (const std::vector<CustomParameterIO>& parameters), (override));
    MOCK_METHOD(OpStatus, CustomParameterRead, (std::vector<CustomParameterIO> & parameters), (override));
    MOCK_METHOD(OpStatus,
        ProgramWrite,
        (const char* data, size_t length, int prog_mode, int target, ProgressCallback callback),
        (override));
    MOCK_METHOD(OpStatus, ResetDevice, (int chipSelect), (override));
};

} // namespace lime::testing

#endif // LIME_COMMSMOCK_H
