#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include "limesuite/config.h"

namespace lime {

/** @brief An interface for Serial Peripheral Interface communications */
class LIME_API ISPI
{
  public:
    /**
      @brief Default path for writing/reading registers.
      @param MOSI Main Out Sub In (data output from main).
      @param MISO Main In Sub Out (data output from sub).
      @param count Input/output data length.
      @returns Whether the operation succeedded or not.
     */
    virtual int SPI(const uint32_t* MOSI, uint32_t* MISO, uint32_t count) = 0;

    /**
      @brief Writing/reading registers for specific slave.
      @param spiBusAddress The address of the SPI device.
      @param MOSI Main Out Sub In (data output from main).
      @param MISO Main In Sub Out (data output from sub).
      @param count Input/output data length.
      @returns Whether the operation succeedded or not.
     */
    virtual int SPI(uint32_t spiBusAddress, const uint32_t* MOSI, uint32_t* MISO, uint32_t count) = 0;
};

/** @brief An interface for Inter-Integrated Circuit communications */
class LIME_API II2C
{
  public:
    /*!
      @brief Write to an available Inter-Integrated Circuit slave.
      @param address Inter-Integrated Circuit slave address.
      @param data Output buffer.
      @param length Output data length.
      @return 0 on success.
     */
    virtual int I2CWrite(int address, const uint8_t* data, uint32_t length) = 0;

    /*!
      @brief Read from an available Inter-Integrated Circuit slave.

      Some implementations can combine a write + read transaction.
      If the device contains multiple I2C masters,
      the address bits can encode which master.
      @param address The address of the slave.
      @param [out] dest Buffer to store read data from the slave.
      @param length Number of bytes to read.
      @return 0 on success.
     */
    virtual int I2CRead(int address, uint8_t* dest, uint32_t length) = 0;
};

struct CustomParameterIO {
    int32_t id;
    double value;
    std::string units;
};

/// @brief An interface for general device communications
class IComms : public ISPI
{
  public:
    /// @brief Destroys the interfaced object.
    virtual ~IComms(){};

    /// @brief Reads the current state of the general-purpose input/output pin direction registers.
    /// @param [out] buffer The buffer to write the current state into.
    /// @param bufLength The length of \p buffer.
    /// @return Whether the operation succeeded or not.
    virtual int GPIODirRead(uint8_t* buffer, const size_t bufLength) { return -1; };

    /// @brief Reads the current state of the general-purpose input/output registers.
    /// @param [out] buffer The buffer to write the current state into.
    /// @param bufLength The length of \p buffer.
    /// @return Whether the operation succeeded or not.
    virtual int GPIORead(uint8_t* buffer, const size_t bufLength) { return -1; };

    /// @brief Writes the current state of the general-purpose input/output pin direction registers.
    /// @param buffer The buffer of the state to write.
    /// @param bufLength The length of \p buffer.
    /// @return Whether the operation succeeded or not.
    virtual int GPIODirWrite(const uint8_t* buffer, const size_t bufLength) { return -1; };

    /// @brief Writes the current state of the general-purpose input/output registers.
    /// @param buffer The buffer of the state to write.
    /// @param bufLength The length of \p buffer.
    /// @return Whether the operation succeeded or not.
    virtual int GPIOWrite(const uint8_t* buffer, const size_t bufLength) { return -1; };

    virtual int CustomParameterWrite(const std::vector<CustomParameterIO>& parameters) { return -1; };
    virtual int CustomParameterRead(std::vector<CustomParameterIO>& parameters) { return -1; };

    typedef bool (*ProgressCallback)(size_t bytesSent, size_t bytesTotal, const char* progressMsg); // return true to stop progress
    virtual int ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr)
    {
        return -1;
    }
    virtual int ResetDevice(int chipSelect) { return -1; };
    virtual int MemoryWrite(uint32_t address, const void* data, uint32_t dataLength) { return -1; };
    virtual int MemoryRead(uint32_t address, void* data, uint32_t dataLength) { return -1; };
};

} // namespace lime
