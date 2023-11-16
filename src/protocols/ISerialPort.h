#pragma once

#include <cstddef>
#include <stdint.h>

namespace lime {

class ISerialPort
{
  public:
    virtual int Write(const uint8_t* data, std::size_t length, int timeout_ms) = 0;
    virtual int Read(uint8_t* data, std::size_t length, int timeout_ms) = 0;
};

}