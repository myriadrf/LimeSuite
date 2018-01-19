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

LMS7_LimeSDR::LMS7_LimeSDR(lime::IConnection* conn, LMS7_Device *obj) : LMS7_Device(obj) 
{
    fpga = new lime::FPGA();
    tx_channels.resize(GetNumChannels());
    rx_channels.resize(GetNumChannels());
    
    while (obj && lms_list.size() > 1)
    {
        delete lms_list.back();
        lms_list.pop_back();
    }
    fpga->SetConnection(conn);
    double refClk = fpga->DetectRefClk(100.6e6);
    this->lms_list[0]->SetConnection(conn);
    mStreamers.push_back(new lime::Streamer(fpga,lms_list[0],0));
    lms_list[0]->SetReferenceClk_SX(false, refClk);
    connection = conn;
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
    if ((mode == "FX3_FLASH") || (mode == "FPGA_FLASH"))
        connection->ProgramWrite(nullptr, 0, 0, 1);
    return ret;
}

}






