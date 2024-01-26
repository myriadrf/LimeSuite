#ifndef MCU_FILE_H
#define MCU_FILE_H

#include "limesuite/config.h"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

/** @brief Structure describing a memory block. */
struct MemBlock {
    size_t m_startAddress;
    std::vector<std::byte> m_bytes;
};

class LIME_API MCU_File
{
  public:
    explicit MCU_File(const char* fileName, const char* mode);
    ~MCU_File();

    bool FileOpened();

    // Read binary file
    void ReadBin(unsigned long limit);

    // Read hex file
    void ReadHex(unsigned long limit);

    bool GetByte(const unsigned long address, std::byte& chr);
    bool BitString(const unsigned long address, const unsigned char bits, const bool lEndian, std::string& str);

    FILE* Handle() const { return m_file; };
    std::vector<MemBlock> m_chunks;
    size_t m_top;

  private:
    FILE* m_file;
};

#endif // MCU_FILE_H
