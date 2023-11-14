#include <gmock/gmock.h>

#include "limesuite/IComms.h"

namespace lime::testing {

class ICommsMock : public IComms
{
  public:
    ICommsMock(){};
    MOCK_METHOD(void, SPI, (const uint32_t* MOSI, uint32_t* MISO, uint32_t count), (override));
    MOCK_METHOD(void, SPI, (uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count), (override));
    MOCK_METHOD(int, GPIODirRead, (uint8_t * buffer, const size_t bufLength), (override));
    MOCK_METHOD(int, GPIORead, (uint8_t * buffer, const size_t bufLength), (override));
    MOCK_METHOD(int, GPIODirWrite, (const uint8_t* buffer, const size_t bufLength), (override));
    MOCK_METHOD(int, GPIOWrite, (const uint8_t* buffer, const size_t bufLength), (override));
    MOCK_METHOD(int,
        CustomParameterWrite,
        (const int32_t* ids, const double* values, const size_t count, const std::string& units),
        (override));
    MOCK_METHOD(int, CustomParameterRead, (const int32_t* ids, double* values, const size_t count, std::string* units), (override));
    MOCK_METHOD(
        int, ProgramWrite, (const char* data, size_t length, int prog_mode, int target, ProgressCallback callback), (override));
    MOCK_METHOD(int, ResetDevice, (int chipSelect), (override));
};

} // namespace lime::testing