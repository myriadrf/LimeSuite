#include <gmock/gmock.h>

class FX3_Mock : public FX3
{
  public:
    FX3_Mock(){};
    MOCK_METHOD(int32_t, BulkTransfer, (uint8_t endPoint, uint8_t* data, int length, int32_t timeout_ms), (override));
    MOCK_METHOD(int32_t,
        ControlTransfer,
        (int requestType, int request, int value, int index, uint8_t* data, uint32_t length, int32_t timeout_ms),
        (override));

#ifdef __unix__
    MOCK_METHOD(void, HandleLibusbEvents, (), (override));
#endif
};
