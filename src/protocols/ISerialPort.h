#ifndef LIME_ISERIALPORT_H
#define LIME_ISERIALPORT_H

#include <cstddef>
#include <cstdint>

namespace lime {

/** @brief Interface for directly reading or writing data to/from the device. */
class ISerialPort
{
  public:
    virtual int Write(const uint8_t* data, std::size_t length, int timeout_ms) = 0;
    virtual int Read(uint8_t* data, std::size_t length, int timeout_ms) = 0;
};

} // namespace lime

#endif // LIME_ISERIALPORT_H
