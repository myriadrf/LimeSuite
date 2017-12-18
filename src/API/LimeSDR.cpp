/*
 * File:   LimeSDR.cpp
 * Author: Ignas J
 *
 * Created on September 18, 2016
 */
#include "LimeSDR.h"
#include "FPGA_common.h"

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
    mStreamers.push_back(new lime::Streamer(fpga,lms_list[0]));
    lms_list[0]->SetReferenceClk_SX(false, refClk);
    connection = conn;
}






