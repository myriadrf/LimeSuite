#ifndef LIME_ISERIALPORT_H
#define LIME_ISERIALPORT_H

#include <cstddef>

namespace lime {

/** @brief Interface for directly reading or writing data to/from the device. */
class ISerialPort
{
  public:
    virtual int Write(const std::byte* data, std::size_t length, int timeout_ms) = 0;
    virtual int Read(std::byte* data, std::size_t length, int timeout_ms) = 0;
};

} // namespace lime

#endif // LIME_ISERIALPORT_H
