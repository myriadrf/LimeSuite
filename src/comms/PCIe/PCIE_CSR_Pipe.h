#ifndef PCIE_CSR_PIPE_H
#define PCIE_CSR_PIPE_H

#include "limesuite/IComms.h"
#include "LMS64CProtocol.h"
#include "LitePCIe.h"

namespace lime {

/** @brief An abstract class for interfacing with Control/Status registers (CSR) of a PCIe device. */
class PCIE_CSR_Pipe : public ISerialPort
{
  public:
    /**
    @brief Constructs a new PCIE_CSR_Pipe object.
    
    @param port The LitePCIe port to use with this pipe.
   */
    explicit PCIE_CSR_Pipe(std::shared_ptr<LitePCIe> port);
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override;
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override;

  protected:
    std::shared_ptr<LitePCIe> port;
};

} // namespace lime

#endif // PCIE_CSR_PIPE_H
