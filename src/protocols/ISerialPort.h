#ifndef LIME_ISERIALPORT_H
#define LIME_ISERIALPORT_H

#include <cstddef>
#include <cstdint>

namespace lime {

/** @brief Interface for directly reading or writing data to/from the device. */
class ISerialPort
{
  public:
    /**
    @brief Writes the specified data into whatever device is implementing this interface.
    
    @param data The data to write to the device.
    @param length The length of the data.
    @param timeout_ms The timeout (in ms) to wait until the transfer times out.
    @return The amount of bytes written.
   */
    virtual int Write(const uint8_t* data, std::size_t length, int timeout_ms) = 0;

    /**
      @brief Reads some data from the device.
      
      @param data The buffer in which to store the read data.
      @param length The length of the data to store.
      @param timeout_ms The timeout (in ms) to wait until the transfer times out.
      @return The amount of bytes read.
     */
    virtual int Read(uint8_t* data, std::size_t length, int timeout_ms) = 0;
};

} // namespace lime

#endif // LIME_ISERIALPORT_H
