#include "LMS_StreamBoard.h"
#include "lmsComms.h"
#include <assert.h>
#include <iostream>

/** @brief Configures Stream board FPGA clocks
    @param serPort Communications port to send data
    @param fOutTx_MHz transmitter frequency in MHz
    @param fOutRx_MHz receiver frequency in MHz
    @param phaseShift_deg IQ phase shift in degrees
    @return 0-success, other-failure
*/
LMS_StreamBoard::Status LMS_StreamBoard::ConfigurePLL(LMScomms *serPort, const float fOutTx_MHz, const float fOutRx_MHz, const float phaseShift_deg)
{
    assert(serPort != nullptr);
    if (serPort == NULL)
        return STREAM_BRD_FAILURE;
    if (serPort->IsOpen() == false)
        return STREAM_BRD_FAILURE;
    const float vcoLimits_MHz[2] = { 600, 1300 };
    int M, C;
    const short bufSize = 64;

    float fOut_MHz = fOutTx_MHz;
    float coef = 0.8*vcoLimits_MHz[1] / fOut_MHz;
    M = C = (int)coef;
    int chigh = (((int)coef) / 2) + ((int)(coef) % 2);
    int clow = ((int)coef) / 2;

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_BRDSPI_WR;


    unsigned char outBuffer[64];
    unsigned char inBuffer[64];

    if (fOut_MHz*M > vcoLimits_MHz[0] && fOut_MHz*M < vcoLimits_MHz[1])
    {
        short index = 8;
        
        memset(outBuffer, 0, bufSize);
        outBuffer[0] = CMD_BRDSPI_WR;
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x0F;
        outBuffer[index++] = 0x15; //c4-c2_bypassed
        outBuffer[index++] = 0x01 | ((M % 2 != 0) ? 0x08 : 0x00) | ((C % 2 != 0) ? 0x20 : 0x00); //N_bypassed

        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x08;
        outBuffer[index++] = 1; //N_high_cnt
        outBuffer[index++] = 1;//N_low_cnt
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x09;
        outBuffer[index++] = chigh; //M_high_cnt
        outBuffer[index++] = clow;	 //M_low_cnt
        for (int i = 0; i <= 1; ++i)
        {
            outBuffer[index++] = 0x00;
            outBuffer[index++] = 0x0A + i;
            outBuffer[index++] = chigh; //cX_high_cnt
            outBuffer[index++] = clow;	 //cX_low_cnt
        }

        float Fstep_us = 1 / (8 * fOutTx_MHz*C);
        float Fstep_deg = (360 * Fstep_us) / (1 / fOutTx_MHz);
        short nSteps = phaseShift_deg / Fstep_deg;
        unsigned short reg2 = 0x0400 | (nSteps & 0x3FF);
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x02;
        outBuffer[index++] = (reg2 >> 8);
        outBuffer[index++] = reg2; //phase

        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x03;
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x01;

        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x03;
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x00;

        reg2 = reg2 | 0x800;
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x02;
        outBuffer[index++] = (reg2 >> 8);
        outBuffer[index++] = reg2;
        outBuffer[2] = index / 4;

        serPort->Write(outBuffer, bufSize);
        long toRead = bufSize;
        serPort->Read(inBuffer, toRead);
        if (inBuffer[1] != STATUS_COMPLETED_CMD)
            return STREAM_BRD_FAILURE;
    }
    else
        return STREAM_BRD_FAILURE;

    fOut_MHz = fOutRx_MHz;
    coef = 0.8*vcoLimits_MHz[1] / fOut_MHz;
    M = C = (int)coef;
    chigh = (((int)coef) / 2) + ((int)(coef) % 2);
    clow = ((int)coef) / 2;
    if (fOut_MHz*M > vcoLimits_MHz[0] && fOut_MHz*M < vcoLimits_MHz[1])
    {
        short index = 8;
        memset(outBuffer, 0, bufSize);
        outBuffer[0] = CMD_BRDSPI_WR;
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x0F;
        outBuffer[index++] = 0x15; //c4-c2_bypassed
        outBuffer[index++] = 0x41 | ((M % 2 != 0) ? 0x08 : 0x00) | ((C % 2 != 0) ? 0x20 : 0x00); //N_bypassed, c1 bypassed

        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x08;
        outBuffer[index++] = 1; //N_high_cnt
        outBuffer[index++] = 1;//N_low_cnt
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x09;
        outBuffer[index++] = chigh; //M_high_cnt
        outBuffer[index++] = clow;	 //M_low_cnt
        for (int i = 0; i <= 1; ++i)
        {
            outBuffer[index++] = 0x00;
            outBuffer[index++] = 0x0A + i;
            outBuffer[index++] = chigh; //cX_high_cnt
            outBuffer[index++] = clow;	 //cX_low_cnt
        }

        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x03;
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x02;

        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x03;
        outBuffer[index++] = 0x00;
        outBuffer[index++] = 0x00;

        outBuffer[2] = index / 4;

        serPort->Write(outBuffer, bufSize);
        long toRead = bufSize;
        serPort->Read(inBuffer, toRead);
        if (inBuffer[1] != STATUS_COMPLETED_CMD)
            return STREAM_BRD_FAILURE;
    }
    else
        return STREAM_BRD_FAILURE;
    return STREAM_BRD_SUCCESS;
}

/** @brief Captures IQ samples from Stream board
    @param isamples destination array for I samples, must be big enough to contain samplesCount
    @param qsamples destination array for Q samples, must be big enough to contain samplesCount
    @param framesCount number of IQ frames to capture
    @param frameStart frame start indicator 0 or 1
    @return 0-success, other-failure
*/
LMS_StreamBoard::Status LMS_StreamBoard::CaptureIQSamples(LMScomms *dataPort, int16_t *isamples, int16_t *qsamples, const uint32_t framesCount, const bool frameStart)
{
    assert(dataPort != nullptr);
    if (dataPort == NULL)
        return STREAM_BRD_FAILURE;
    if (dataPort->IsOpen() == false)
        return STREAM_BRD_FAILURE;

    int16_t sample_value;
    const uint32_t bufSize = framesCount * 2 * sizeof(uint16_t);
    char *buffer = new char[bufSize];
    if (buffer == 0)
    {
#ifndef NDEBUG
        std::cout << "Failed to allocate memory for samples buffer" << std::endl;
#endif
        return STREAM_BRD_FAILURE;
    }
    memset(buffer, 0, bufSize);

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_BRDSPI_RD;
    pkt.outBuffer.push_back(0x00);
    pkt.outBuffer.push_back(0x05);
    dataPort->TransferPacket(pkt);
    if (pkt.status != STATUS_COMPLETED_CMD)
        return STREAM_BRD_FAILURE;

    uint16_t regVal = (pkt.inBuffer[2] * 256) + pkt.inBuffer[3];
    pkt.cmd = CMD_BRDSPI_WR;
    pkt.outBuffer.clear();
    pkt.outBuffer.push_back(0x00);
    pkt.outBuffer.push_back(0x05);
    pkt.outBuffer.push_back(0);
    pkt.outBuffer.push_back(regVal | 0x4);    
    dataPort->TransferPacket(pkt);
    if (pkt.status != STATUS_COMPLETED_CMD)
        return STREAM_BRD_FAILURE;

    int bytesReceived = 0;
    for(int i = 0; i<3; ++i)
        bytesReceived = dataPort->ReadStream(buffer, bufSize, 5000);
    if (bytesReceived > 0)
    {
        bool iqSelect = false;
        int16_t frameCounter = 0;        
        for (uint32_t b = 0; b < bufSize;)
        {
            sample_value = buffer[b++] & 0xFF;
            sample_value |= (buffer[b++] & 0x0F) << 8;
            sample_value = sample_value << 4; //shift left then right to fill sign bits
            sample_value = sample_value >> 4;
            if (iqSelect == false)
                isamples[frameCounter] = sample_value;
            else
                qsamples[frameCounter] = sample_value;
            frameCounter += iqSelect;
            iqSelect = !iqSelect;
        }        
    }
    pkt.cmd = CMD_BRDSPI_RD;
    pkt.outBuffer.clear();
    pkt.outBuffer.push_back(0x00);
    pkt.outBuffer.push_back(0x01);
    pkt.outBuffer.push_back(0x00);
    pkt.outBuffer.push_back(0x05);    
    dataPort->TransferPacket(pkt);

    regVal = (pkt.inBuffer[2] * 256) + pkt.inBuffer[3];
    pkt.cmd = CMD_BRDSPI_WR;
    pkt.outBuffer.clear();
    pkt.outBuffer.push_back(0x00);
    pkt.outBuffer.push_back(0x05);
    pkt.outBuffer.push_back(0);
    pkt.outBuffer.push_back(regVal & ~0x4);    
    dataPort->TransferPacket(pkt);

    delete[] buffer;
    return STREAM_BRD_SUCCESS;
}