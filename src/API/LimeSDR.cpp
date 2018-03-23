/*
 * File:   LimeSDR.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR.h"
#include "FPGA_common.h"
#include "device_constants.h"

namespace lime
{

LMS7_LimeSDR::LMS7_LimeSDR(lime::IConnection* conn, LMS7_Device *obj) : LMS7_Generic(conn, obj)
{
}

std::vector<std::string> LMS7_LimeSDR::GetProgramModes() const
{
    return {program_mode::autoUpdate,
            program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::fx3Flash, program_mode::fx3Reset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

int LMS7_LimeSDR::Program(const std::string& mode, const char* data, size_t len, lime::IConnection::ProgrammingCallback callback) const
{
    int ret = LMS7_Device::Program(mode, data, len, callback);
    
    if ((mode == program_mode::fx3Flash) || (mode == program_mode::fpgaFlash))
        connection->ProgramWrite(nullptr, 0, 0, 1, nullptr);
    return ret;
}

LMS7_LimeSDR_PCIE::LMS7_LimeSDR_PCIE(lime::IConnection* conn, LMS7_Device *obj) : LMS7_Generic(conn, obj)
{
}

std::vector<std::string> LMS7_LimeSDR_PCIE::GetProgramModes() const
{
    return {program_mode::fpgaFlash, program_mode::fpgaReset,
            program_mode::mcuRAM, program_mode::mcuEEPROM, program_mode::mcuReset};
}

}






