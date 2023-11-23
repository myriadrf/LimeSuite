#ifndef LIME_FT601MOCK_H
#define LIME_FT601MOCK_H

#include <gmock/gmock.h>

#include "FT601/FT601.h"

namespace lime::testing {

class FT601Mock : public FT601
{
  public:
    MOCK_METHOD(bool, Connect, (uint16_t vid, uint16_t pid, const std::string& serial), (override));
    MOCK_METHOD(void, Disconnect, (), (override));

    MOCK_METHOD(int32_t, BulkTransfer, (uint8_t endPoint, uint8_t* data, int length, int32_t timeout_ms), (override));
};

} // namespace lime::testing

#endif // LIME_FT601MOCK_H
