#ifndef LIME_FX3MOCK_H
#define LIME_FX3MOCK_H

#include <gmock/gmock.h>

#include "FX3/FX3.h"

namespace lime::testing {

class FX3Mock : public FX3
{
  public:
    MOCK_METHOD(bool, Connect, (uint16_t vid, uint16_t pid, const std::string& serial), (override));
    MOCK_METHOD(void, Disconnect, (), (override));

    MOCK_METHOD(int32_t, BulkTransfer, (uint8_t endPoint, std::byte* data, int length, int32_t timeout_ms), (override));
    MOCK_METHOD(int32_t,
        ControlTransfer,
        (int requestType, int request, int value, int index, std::byte* data, uint32_t length, int32_t timeout_ms),
        (override));

#ifdef __unix__
    MOCK_METHOD(void, HandleLibusbEvents, (), (override));
#endif
};

} // namespace lime::testing

#endif // LIME_FX3MOCK_H
