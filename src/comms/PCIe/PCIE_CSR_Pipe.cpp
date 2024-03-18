#include "PCIE_CSR_Pipe.h"

using namespace lime;

PCIE_CSR_Pipe::PCIE_CSR_Pipe(std::shared_ptr<LitePCIe> port)
    : port(port)
{
}

int PCIE_CSR_Pipe::Write(const uint8_t* data, size_t length, int timeout_ms)
{
    return port->WriteControl(data, length, timeout_ms);
}
int PCIE_CSR_Pipe::Read(uint8_t* data, size_t length, int timeout_ms)
{
    return port->ReadControl(data, length, timeout_ms);
}
