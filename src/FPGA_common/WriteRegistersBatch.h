#ifndef LIME_WRITEREGISTERSBATCH_H
#define LIME_WRITEREGISTERSBATCH_H

#include <cstdint>
#include <vector>

#include "FPGA_common.h"

namespace lime {

/** @brief A class for writing a batch of registers into the FPGA. */
class WriteRegistersBatch
{
  public:
    WriteRegistersBatch(FPGA* fpga);
    ~WriteRegistersBatch();

    OpStatus Flush();
    void WriteRegister(uint16_t addr, uint16_t value);

  private:
    FPGA* owner;
    std::vector<uint32_t> addrs;
    std::vector<uint32_t> values;
};

} // namespace lime

#endif // LIME_WRITEREGISTERSBATCH_H
