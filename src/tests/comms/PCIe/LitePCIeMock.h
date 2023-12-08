#ifndef LIME_LITEPCIEMOCK_H
#define LIME_LITEPCIEMOCK_H

#include <gmock/gmock.h>

#include "LitePCIe.h"

namespace lime::testing {

class LitePCIeMock : public LitePCIe
{
  public:
    MOCK_METHOD(int, WriteControl, (const uint8_t* buffer, int length, int timeout_ms), (override));
    MOCK_METHOD(int, ReadControl, (uint8_t * buffer, int length, int timeout_ms), (override));
};

} // namespace lime::testing

#endif // LIME_LITEPCIEMOCK_H
