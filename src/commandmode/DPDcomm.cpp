/* --------------------------------------------------------------------------------------------
FILE:		DPDcomm.cpp
DESCRIPTION     Quadrature Adaptive Digital Predistorter API                
CONTENT:
AUTHOR:		Lime Microsystems LTD
DATE:		Jan 01, 2018
-------------------------------------------------------------------------------------------- */

#include "DPDcomm.h"
#include "nrc.h"
#include "math.h"
#include <iostream>
using namespace lime;
#include "iniParser.h"

const char *config_filename = "QADPDconfig.ini";

DPDcomm::DPDcomm()
{

    chA = 1;
    range = 16.0;
    samplesReceived = 0;
    ind = 0;
    m_iDPDchA = 0;
    m_iDPDchB = 0;
    QADPD_N = 4;
    QADPD_M = 2;
    QADPD_ND = 10;
    QADPD_AM = 14;
    QADPD_SKIP = 100;
    QADPD_GAIN = 1.0;
    QADPD_LAMBDA = 0.9998;
    QADPD_YPFPGA = true;
    QADPD_UPDATE = 15000;
    xp_samples = NULL;
    yp_samples = NULL;
    x_samples = NULL;
    y_samples = NULL;
    u_samples = NULL;
    error_samples = NULL;
    lmsControl = NULL;
    gain_chA = 1.0;
    gain_chB = 1.0;
    ND_chA = 10;
    ND_chB = 10;

    CreateArrays();
    for (int i = 0; i <= 4; i++)
    {
        for (int j = 0; j <= 3; j++)
        {
            coeffa_chA[i][j] = 0.0;
            coeffb_chA[i][j] = 0.0;
            coeffa_chB[i][j] = 0.0;
            coeffb_chB[i][j] = 0.0;
        }
    }
    chA = 1;
    //SelectSource(chA);
}

DPDcomm::~DPDcomm()
{
}

void DPDcomm::Connect(lms_device_t *device)
{
    lmsControl = device;
}

void DPDcomm::Init(int N, int M)
{

    if ((N >= 1) && (N <= 4))
        QADPD_N = N;
    else
    {
        QADPD_N = 4;
        std::cout << "N = [1..4]. Default value selected N=4" << std::endl;
    }
    if ((M >= 1) && (M <= 3))
        QADPD_M = M;
    else
    {
        QADPD_M = 2;
        std::cout << "M = [1,3]. Default value selected M=2" << std::endl;
    }
    Qadpd = new qadpd(QADPD_N, QADPD_M, QADPD_ND);
}

void DPDcomm::SetLambda(double val)
{

    if ((val >= 0.0) && (val <= 1.0))
        QADPD_LAMBDA = val;
    else
    {
        QADPD_LAMBDA = 0.9998;
        std::cout << "Lambda = [0..1]. Default value selected Lambda=0.998" << std::endl;
    }
}

void DPDcomm::SetND(int val)
{
    if ((val >= 0) && (val <= 300))
        QADPD_ND = val;
    else
    {
        QADPD_ND = 100;
        std::cout << "ND = [0..300]. Default value selected ND=100" << std::endl;
    }

    if (chA == 1)
    {
        ND_chA = QADPD_ND;
    }
    else
    {
        ND_chB = QADPD_ND;
    }
}

void DPDcomm::SetGain(double val)
{
    if ((val >= 0.333) && (val <= 6.0))
    {
        QADPD_GAIN = val;
    }
    else
    {
        QADPD_GAIN = 1.0;
        std::cout << "Gain = [0.333..6.0]. Default value selected Gain=1.0" << std::endl;
    }
    if (chA == 1)
    {
        gain_chA = QADPD_GAIN;
    }
    else
    {
        gain_chB = QADPD_GAIN;
    }
}

// PA control
void DPDcomm::PAchA(int val)
{

    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;

    int lsb = 2;
    int msb = 2;
    uint16_t code = 0x0000;

    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= ~mask;

    if (val == 1)
        code = 1;
    regValue |= (code << lsb) & mask;
    if (LMS_WriteFPGAReg(lmsControl, maddress, regValue) != 0)
        ret = -1;
}

int DPDcomm::isOnPAchA()
{
    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;
    int lsb = 2;
    int msb = 2;
    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= mask;
    regValue >>= lsb;
    return (int)regValue;
}

void DPDcomm::PAchB(int val)
{

    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;

    int lsb = 3;
    int msb = 3;
    uint16_t code = 0x0000;
    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= ~mask;

    if (val == 1)
        code = 1;
    regValue |= (code << lsb) & mask;
    if (LMS_WriteFPGAReg(lmsControl, maddress, regValue) != 0)
        ret = -1;
}

int DPDcomm::isOnPAchB()
{
    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;
    int lsb = 3;
    int msb = 3;
    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= mask;
    regValue >>= lsb;
    return (int)regValue;
}

// DC-DC control
void DPDcomm::DCDCchA(int val)
{

    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;

    int lsb = 7;
    int msb = 7;
    uint16_t code = 0x0000;

    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= ~mask;

    if (val == 1)
        code = 1;
    regValue |= (code << lsb) & mask;
    if (LMS_WriteFPGAReg(lmsControl, maddress, regValue) != 0)
        ret = -1;
}

int DPDcomm::isOnDCDCchA()
{
    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;
    int lsb = 7;
    int msb = 7;
    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= mask;
    regValue >>= lsb;
    return (int)regValue;
}

void DPDcomm::DCDCchB(int val)
{

    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;
    int lsb = 8;
    int msb = 8;
    uint16_t code = 0x0000;
    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= ~mask;
    if (val == 1)
        code = 1;
    regValue |= (code << lsb) & mask;
    if (LMS_WriteFPGAReg(lmsControl, maddress, regValue) != 0)
        ret = -1;
}

int DPDcomm::isOnDCDCchB()
{
    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;
    int lsb = 8;
    int msb = 8;
    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= mask;
    regValue >>= lsb;
    return (int)regValue;
}

int DPDcomm::ReadDataQPCIe()
{
    int status = 0;
    const int bytesToRead = 20 * 8192 * 2;
    unsigned char *buffer = new unsigned char[bytesToRead];
    memset(buffer, 0, bytesToRead);
    auto conn = ((LMS7_Device *)lmsControl)->GetConnection();
    if (!conn || !conn->IsOpen())
    {
        std::cout << "[ERROR] Connection not initialized\n";
        return 0;
    }
    else
    {

        int bytesReceived = ((LMS7_Device *)lmsControl)->GetConnection()->ReadDPDBuffer((char *)buffer, bytesToRead);

        if (bytesReceived == bytesToRead)
        {
            int samplesReceived = bytesReceived / 20;
            //allocate memory for FFT
            int16_t value;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i]) & 0xFF;
                value |= (buffer[4 * i + 1] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    xp_samples[i].r = value;
                }
                value = (buffer[4 * i + 2]) & 0xFF;
                value |= (buffer[4 * i + 3] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    xp_samples[i].i = value;
                }
            }

            unsigned offset = bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    yp_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA == 1)
                {
                    yp_samples[i].i = value;
                }
            }

            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                x_samples[i].r = (int)(QADPD_GAIN * value);
                value;

                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                x_samples[i].i = (int)(QADPD_GAIN * value);
                value;
            }
            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    xp_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    xp_samples[i].i = value;
                }
            }

            offset += bytesReceived / 5;
            for (int i = 0; i < samplesReceived; i++)
            {
                value = (buffer[4 * i + offset]) & 0xFF;
                value |= (buffer[4 * i + 1 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    yp_samples[i].r = value;
                }
                value = (buffer[4 * i + 2 + offset]) & 0xFF;
                value |= (buffer[4 * i + 3 + offset] << 8) & 0xFF00;
                value >>= 2;
                if (chA != 1)
                {
                    yp_samples[i].i = value;
                }
            }

            for (int i = 0; i < samplesReceived; i++)
            {
                y_samples[i].r = 0;
                y_samples[i].i = 0;
                error_samples[i].r = 0;
                error_samples[i].i = 0;
                u_samples[i].r = 0;
                u_samples[i].i = 0;
            }
        }
        else
            std::cout << "[ERROR] DPD received samples " << bytesReceived << "/" << bytesToRead << std::endl;
    }
    samplesReceived = 2 * 8192;
    QADPD_SKIP = 0;
    delete buffer;
    buffer = NULL;
    return status;
}

void DPDcomm::CreateArrays()
{

    if (xp_samples != NULL)
        delete[] xp_samples;
    if (yp_samples != NULL)
        delete[] yp_samples;
    if (x_samples != NULL)
        delete[] x_samples;
    if (y_samples != NULL)
        delete[] y_samples;
    if (u_samples != NULL)
        delete[] u_samples;
    if (error_samples != NULL)
        delete[] error_samples;
    int samplesReceived = 32768;
    xp_samples = new kiss_fft_cpx[samplesReceived];
    yp_samples = new kiss_fft_cpx[samplesReceived];
    x_samples = new kiss_fft_cpx[samplesReceived];
    y_samples = new kiss_fft_cpx[samplesReceived];
    u_samples = new kiss_fft_cpx[samplesReceived];
    error_samples = new kiss_fft_cpx[samplesReceived];
    for (int i = 0; i < 32768; i++)
    {
        xp_samples[i].i = 0;
        xp_samples[i].r = 0;
        yp_samples[i].i = 0;
        yp_samples[i].r = 0;
        x_samples[i].i = 0;
        x_samples[i].r = 0;
        y_samples[i].i = 0;
        y_samples[i].r = 0;
        u_samples[i].i = 0;
        u_samples[i].r = 0;
        error_samples[i].i = 0;
        error_samples[i].r = 0;
    }
}

void DPDcomm::CalculateXError(int log1)
{

    double tempi, tempr, tempi2, tempr2, error, error2 = 0.0;
    double Am = (1 << (QADPD_AM - 1));
    int off = QADPD_ND + 8;
    double minerror = 100000.0;

    for (int i = 300; i < samplesReceived; i++)
    {
        tempi2 = (xp_samples[i - off].i) / Am;
        tempr2 = (xp_samples[i - off].r) / Am;
        error2 += (tempi2 * tempi2 + tempr2 * tempr2);
    }

    for (off = QADPD_ND + 6; off <= QADPD_ND + 11; off++)
    {
        error = 0.0;
        for (int i = 300; i < samplesReceived; i++)
        {
            tempi = (xp_samples[i - off].i - x_samples[i].i) / Am;
            tempr = (xp_samples[i - off].r - x_samples[i].r) / Am;
            error += (tempi * tempi + tempr * tempr);
        }
        if (error < minerror)
            minerror = error;
    }

    if (error2 > 0)
        minerror = sqrt(minerror / error2);
    else
        minerror = 0.0;

    if (log1 == 1)
        if (chA == 1)
            std::cout << "[INFO] Ch.A e=" << minerror << std::endl;
        else
            std::cout << "[INFO] Ch.B e=" << minerror << std::endl;
}

double DPDcomm::CalculateGainCorr(double p1, double p2, double g1, double g2, double p)
{

    double g = 0.0;
    if (p < p1)
        g = g1;
    else if (p > p2)
        g = g2;
    else
    {
        g = g1 + (g2 - g1) / (p2 - p1) * (p - p1);
    }
    return g;
}

double DPDcomm::CalculateSignalPower(int Size, double *a, double *b, double Am)
{

    int offset = 100;
    int i = 0;
    double sum = 0;
    if (Size <= 0)
        return 0;
    for (i = offset; i < Size; i++)
        sum = sum + (a[i] * a[i] + b[i] * b[i]) / (Am * Am);
    sum = sum / Size;
    return sum;
}

void DPDcomm::CalculateND(int Size, double *a, double *b, int *pND)
{

    int ND_limit = 0;
    int offset = 0;
    int size = 0;
    int j = 0;
    int ND = 0;
    int iND = 0;
    double corr = 0.0;
    double max_corr = 0.0;
    if (Size <= 0)
        return;

    ND_limit = 300;
    offset = 200;
    size = Size - ND_limit;

    for (iND = 0; iND <= ND_limit; iND++)
    {
        corr = 0.0;
        for (j = offset; j < size; j++)
            corr = corr + abs(a[j] * b[j - iND]);
        if (corr > max_corr)
        {
            max_corr = corr;
            ND = iND;
        }
    }

    *pND = ND;
}
double DPDcomm::CalculateInputPower()
{
    int i = 0;
    double pow_u = 0;
    double *b = new double[32768];
    double *d = new double[32768];

    for (i = 0; i < 32768; i++)
    {
        b[i] = d[i] = 0.0;
    }

    RunQADPD();
    for (int i = 0; i < samplesReceived; i++)
    {
        b[i] = (double)u_samples[i].r;
        d[i] = (double)u_samples[i].i;
    }

    pow_u = CalculateSignalPower(samplesReceived, b, d, (1 << (QADPD_AM - 1)));
    return pow_u;
}

void DPDcomm::CalculateErrorPower()
{
    int i = 0;
    double pow_uy = 0;
    double *a = new double[32768];
    double *c = new double[32768];

    for (i = 0; i < 32768; i++)
    {
        a[i] = c[i] = 0.0;
    }

    for (int i = 0; i < samplesReceived; i++)
    {
        a[i] = (double)y_samples[i].r - u_samples[i].r;
        c[i] = (double)y_samples[i].i - u_samples[i].i;
    }

    pow_uy = CalculateSignalPower(samplesReceived, a, c, (1 << (QADPD_AM - 1)));
    std::cout << "[INFO] Power of output signal:" << pow_uy << std::endl;
}

void DPDcomm::CalibrateND()
{

    long temp = 0;
    double tempd = 0.0;
    int i = 0;
    double *a = new double[32768];
    double *b = new double[32768];
    double *c = new double[32768];
    double *d = new double[32768];

    for (i = 0; i < 32768; i++)
    {
        a[i] = b[i] = c[i] = d[i] = 0.0;
    }

    int nd = 0;
    double pow_u = 0;
    double pow_y = 0;
    double gain = 1.000;
    double gainCorr = 1.0;
    CreateArrays();
    QADPD_SKIP = 100;
    QADPD_ND = 10;
    QADPD_GAIN = 1.0;
    QADPD_YPFPGA = true;
    std::cout << "[INFO] start ND calibration\n";

    Qadpd->init(QADPD_N, QADPD_M, 10, 1.0, QADPD_LAMBDA, (1 << (QADPD_AM - 1)), QADPD_SKIP);
    std::cout << "[INFO] Qadpd initialized\n";
    Qadpd->reset_coeff();
    std::cout << "[INFO] Qadpd coeff. reset\n";
    SendCoef();
    ReadDataQPCIe();
    RunQADPD();
    for (int i = 0; i < samplesReceived; i++)
    {
        a[i] = (double)y_samples[i].r;
        b[i] = (double)u_samples[i].r;
        c[i] = (double)y_samples[i].i;
        d[i] = (double)u_samples[i].i;
    }

    CalculateND(samplesReceived, a, b, &nd);
    std::cout << "[INFO] ND calibration result: QADPD_ND=" << nd + 10 + 1 << std::endl;
    QADPD_ND = nd + 10 + 1;

    if (QADPD_ND > 200)
    {
        std::cout << "[WARN] ND cannot be calculated, try to calibrate ND again." << std::endl;
    }
    if (chA == 1)
    {
        ND_chA = QADPD_ND;
    }
    else
    {
        ND_chB = QADPD_ND;
    }
    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;
}

void DPDcomm::CalibrateGain()
{

    long temp = 0;
    double tempd = 0.0;
    int i = 0;
    double *a = new double[32768];
    double *b = new double[32768];
    double *c = new double[32768];
    double *d = new double[32768];
    int nd = 0;
    double pow_u = 0;
    double pow_y = 0;
    double gain = 1.000;
    double gainCorr = 1.0;
    CreateArrays();
    QADPD_SKIP = 100;
    QADPD_GAIN = 1.0;
    QADPD_YPFPGA = true;
    Qadpd->init(QADPD_N, QADPD_M, QADPD_ND, 1.0, QADPD_LAMBDA, (1 << (QADPD_AM - 1)), QADPD_SKIP);
    Qadpd->reset_coeff();
    SendCoef();
    ReadDataQPCIe();
    RunQADPD();

    for (i = 0; i < samplesReceived; i++)
    {
        a[i] = (double)y_samples[i].r;
        b[i] = (double)u_samples[i].r;
        c[i] = (double)y_samples[i].i;
        d[i] = (double)u_samples[i].i;
    }

    pow_y = CalculateSignalPower(samplesReceived, a, c, (1 << (QADPD_AM - 1)));
    pow_u = CalculateSignalPower(samplesReceived, b, d, (1 << (QADPD_AM - 1)));
    gainCorr = CalculateGainCorr(0.03, 0.08, 1.0, 1.0, pow_u); // was 1.41
    gain = sqrt(pow_u / pow_y) * gainCorr;

    std::cout << "[INFO] Signal power correction=" << 1 / gainCorr << std::endl;
    std::cout << "[INFO] QADPD_GAIN=" << gain << std::endl;
    if ((gain > 6.0) || (gain < 0.333))
    {
        gain = 1.0;
        std::cout << "[WARN] Gain cannot be calculated, set to default 1.0" << std::endl;
        std::cout << "[WARN] Change the receiver settings in LimeSuite." << std::endl;
    }

    else if (gain <= 1.0)
    {
        std::cout << "[WARN] Gain should be greater than 1.0." << std::endl;
        std::cout << "[WARN] Change the receiver settings in LimeSuite." << std::endl;
    }
    QADPD_GAIN = gain;
    if (chA == 1)
    {
        gain_chA = QADPD_GAIN;
    }
    else
    {
        gain_chB = QADPD_GAIN;
    }
    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;
}

void DPDcomm::RunQADPD()
{

    Qadpd->start();
    Qadpd->prepare();
    for (int i = 3; i < samplesReceived - 3; i++)
    {

        Qadpd->oeval(xp_samples[i].r, xp_samples[i].i, x_samples[i + ind].r,
                     x_samples[i + ind].i, yp_samples[i].r, yp_samples[i].i, QADPD_YPFPGA);
        y_samples[i].r = Qadpd->yI;
        y_samples[i].i = Qadpd->yQ;
        u_samples[i].r = Qadpd->uI;
        u_samples[i].i = Qadpd->uQ;
        error_samples[i].r = Qadpd->uI - Qadpd->yI;
        error_samples[i].i = Qadpd->uQ - Qadpd->yQ;
    }
    Qadpd->finish();
}

void DPDcomm::ResetCoef()
{

    Qadpd->init(QADPD_N, QADPD_M, QADPD_ND, QADPD_GAIN, QADPD_LAMBDA, (1 << (QADPD_AM - 1)), QADPD_SKIP);
    Qadpd->reset_coeff();
    std::cout << "[INFO] Qadpd coeff. reset\n";
    SendCoef();
}

void DPDcomm::SendCoef()
{

    range = 16.0;
    double am = 32768.0 / range;
    int temp = 0;

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0x0000);
    else
        SPI_write(lmsControl, 0x0003, 0x0000);
    SPI_write(lmsControl, 0x0004, 0x0000);

    double a1, b1, c1, d1 = 0.0;
    double oi1, oq1 = 0.0;
    short tempsh = 0;
    short tempsh2 = 0;
    double eps = 0.002;
    int i = 0;
    int j = 0;
    int LIM2 = 1;
    int temp_i = 0;

    double K = 0.0; //

    for (i = 0; i <= 4; i++)
    {
        for (j = 0; j <= 3; j++)
        {

            K = (double)pow(2, j);

            if ((i <= QADPD_N) && (j <= QADPD_M))
            {
                a1 = Qadpd->a[i][j];
                b1 = Qadpd->b[i][j];

                if (a1 >= range * K)
                {
                    a1 = (range * K - eps);
                    Qadpd->a[i][j] = a1;
                }
                if (a1 <= (-range * K))
                {
                    a1 = (-range * K + eps);
                    Qadpd->a[i][j] = a1;
                }
                if (b1 >= range * K)
                {
                    b1 = range * K - eps;
                    Qadpd->b[i][j] = b1;
                }
                if (b1 <= (-range * K))
                {
                    b1 = (-range * K + eps);
                    Qadpd->b[i][j] = b1;
                }
            }
            else
            {
                a1 = 0.0;
                b1 = 0.0;
            }

            if (chA == 1)
            {
                coeffa_chA[i][j] = a1;
                coeffb_chA[i][j] = b1;
            }
            else
            {
                coeffa_chB[i][j] = a1;
                coeffb_chB[i][j] = b1;
            }

            temp_i = (int)(am * a1 * 4.0 / K + 0.5); //
            tempsh = (short)(temp_i >> 2);
            tempsh2 = (short)((temp_i & 0x0003) << 8);
            temp = 0x1000 + tempsh2 + i * 16 + j;

            SPI_write(lmsControl, 0x0004, tempsh);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, temp);
            else
                SPI_write(lmsControl, 0x0003, temp);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, 0x0000);
            else
                SPI_write(lmsControl, 0x0003, 0x0000);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            temp_i = (int)(am * b1 * 4.0 / K + 0.5); //
            tempsh = (short)(temp_i >> 2);
            tempsh2 = (short)((temp_i & 0x0003) << 8);

            temp = 0x2000 + tempsh2 + i * 16 + j;

            SPI_write(lmsControl, 0x0004, tempsh);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, temp);
            else
                SPI_write(lmsControl, 0x0003, temp);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, 0x0000);
            else
                SPI_write(lmsControl, 0x0003, 0x0000);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;
        }
    }

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0xF000);
    else
        SPI_write(lmsControl, 0x0003, 0xF000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ;

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0x0000);
    else
        SPI_write(lmsControl, 0x0003, 0x0000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ;

    SPI_write(lmsControl, 0x0004, 0x0000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ;
}

void DPDcomm::UpdateCoef()
{

    range = 16.0;
    double am = 32768.0 / range;
    int temp = 0;
    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0x0000);
    else
        SPI_write(lmsControl, 0x0003, 0x0000);
    SPI_write(lmsControl, 0x0004, 0x0000);

    double a1, b1, c1, d1 = 0.0;
    double oi1, oq1 = 0.0;

    short tempsh = 0;
    short tempsh2 = 0;
    double eps = 0.002;
    int i = 0;
    int j = 0;
    int LIM2 = 1;
    int temp_i = 0;
    double K = 0.0; //

    for (i = 0; i <= 4; i++)
    {
        for (j = 0; j <= 3; j++)
        {

            K = (double)pow(2, j);

            if (chA == 1)
            {
                a1 = coeffa_chA[i][j];
                b1 = coeffb_chA[i][j];
            }
            else
            {
                a1 = coeffa_chB[i][j];
                b1 = coeffb_chB[i][j];
            }

            temp_i = (int)(am * a1 * 4.0 / K + 0.5);
            tempsh = (short)(temp_i >> 2);
            tempsh2 = (short)((temp_i & 0x0003) << 8);

            temp = 0x1000 + tempsh2 + i * 16 + j;

            SPI_write(lmsControl, 0x0004, tempsh);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, temp);
            else
                SPI_write(lmsControl, 0x0003, temp);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, 0x0000);
            else
                SPI_write(lmsControl, 0x0003, 0x0000);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            temp_i = (int)(am * b1 * 4.0 / K + 0.5);
            tempsh = (short)(temp_i >> 2);
            tempsh2 = (short)((temp_i & 0x0003) << 8);
            temp = 0x2000 + tempsh2 + i * 16 + j;

            SPI_write(lmsControl, 0x0004, tempsh);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, temp);
            else
                SPI_write(lmsControl, 0x0003, temp);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;

            if (chA == 1)
                SPI_write(lmsControl, 0x0002, 0x0000);
            else
                SPI_write(lmsControl, 0x0003, 0x0000);
            for (int k = 0; k <= LIM2 * 100; k++)
                ;
        }
    }
    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0xF000);
    else
        SPI_write(lmsControl, 0x0003, 0xF000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ;

    if (chA == 1)
        SPI_write(lmsControl, 0x0002, 0x0000);
    else
        SPI_write(lmsControl, 0x0003, 0x0000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ;

    SPI_write(lmsControl, 0x0004, 0x0000);
    for (int k = 0; k <= LIM2 * 100; k++)
        ;
}

int DPDcomm::Prepare_train()
{

    double error_nd_m5 = 0.0;
    double error_nd_m4 = 0.0;
    double error_nd_m3 = 0.0;
    double error_nd_m2 = 0.0;
    double error_nd_m1 = 0.0;
    double error_nd_p0 = 0.0;
    double error_nd_p1 = 0.0;
    double error_nd_p2 = 0.0;
    double error_nd_p3 = 0.0;
    double error_nd_p4 = 0.0;
    double error_nd_p5 = 0.0;
    int SKIP = 300;
    int LIMIT = samplesReceived - 100;
    int counter_e = 0;

    double xpr = 0.0;
    double xpi = 0.0;

    double ypr = 0.0;
    double ypi = 0.0;

    kiss_fft_cpx *xp;
    kiss_fft_cpx *yp;
    xp = xp_samples;
    yp = yp_samples;

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 5; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 5].r, x_samples[i + 5].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p5 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 4; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 4].r, x_samples[i + 4].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p4 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 3; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 3].r, x_samples[i + 3].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p3 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 2; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 2].r, x_samples[i + 2].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p2 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived - 1; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i + 1].r, x_samples[i + 1].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p1 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 0; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i].r, x_samples[i].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_p0 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 1; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 1].r, x_samples[i - 1].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m1 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 2; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 2].r, x_samples[i - 2].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m2 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 3; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 3].r, x_samples[i - 3].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m3 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 4; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 4].r, x_samples[i - 4].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m4 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    counter_e = 0;
    for (int i = 5; i < samplesReceived; i++)
    {
        Qadpd->oeval(xp[i].r, xp[i].i, x_samples[i - 5].r, x_samples[i - 5].i, yp[i].r, yp[i].i, QADPD_YPFPGA);
        if ((i > SKIP) && (counter_e < LIMIT))
        {
            error_nd_m5 += Qadpd->err;
            counter_e++;
        }
        if (counter_e >= LIMIT)
            i = samplesReceived;
    }

    int ind = 0;
    double minim = 0.0;

    if (error_nd_p5 < error_nd_p4)
    {
        ind = 5;
        minim = error_nd_p5;
    }
    else
    {
        ind = 4;
        minim = error_nd_p4;
    }

    if (minim > error_nd_p3)
    {
        ind = 3;
        minim = error_nd_p3;
    }
    if (minim > error_nd_p2)
    {
        ind = 2;
        minim = error_nd_p2;
    }

    if (minim > error_nd_p1)
    {
        ind = 1;
        minim = error_nd_p1;
    }

    if (minim > error_nd_p0)
    {
        ind = 0;
        minim = error_nd_p0;
    }

    if (minim > error_nd_m1)
    {
        ind = -1;
        minim = error_nd_m1;
    }

    if (minim > error_nd_m2)
    {
        ind = -2;
        minim = error_nd_m2;
    }

    if (minim > error_nd_m3)
    {
        ind = -3;
        minim = error_nd_m3;
    }

    if (minim > error_nd_m4)
    {
        ind = -4;
        minim = error_nd_m4;
    }

    if (minim > error_nd_m5)
    {
        ind = -5;
        minim = error_nd_m5;
    }
    return (ind);
}

void DPDcomm::SaveWfm()
{

    int i, q, k, j;
    std::string fname;
    FILE *fp;

    int Limit = 307200;

    fname = "xp.wfm"; // Output file name is xp.wfm
    fp = fopen(fname.c_str(), "w");
    j = 0;
    k = 0;
    while (j < Limit)
    {
        i = (int)(xp_samples[k].r * 4.0);
        q = (int)(xp_samples[k].i * 4.0);
        fprintf(fp, "%c%c%c%c", 0xFF & (i >> 8), 0xFF & (i), 0xFF & (q >> 8), 0xFF & (q));
        if (k >= 16383)
            k = 0;
        else
            k++;
        j++;
    }
    fclose(fp);

    fname = "yp.wfm"; // Output file name is yp.wfm
    fp = fopen(fname.c_str(), "w");
    j = 0;
    k = 0;
    while (j < Limit)
    {
        i = (int)(yp_samples[k].r * 4.0);
        q = (int)(yp_samples[k].i * 4.0);
        fprintf(fp, "%c%c%c%c", 0xFF & (i >> 8), 0xFF & (i), 0xFF & (q >> 8), 0xFF & (q));
        if (k >= 16383)
            k = 0;
        else
            k++;
        j++;
    }
    fclose(fp);

    fname = "x.wfm"; // Output file name is x.wfm
    fp = fopen(fname.c_str(), "w");
    j = 0;
    k = 0;
    while (j < Limit)
    {
        i = (int)(x_samples[k].r * 4.0);
        q = (int)(x_samples[k].i * 4.0);
        fprintf(fp, "%c%c%c%c", 0xFF & (i >> 8), 0xFF & (i), 0xFF & (q >> 8), 0xFF & (q));
        if (k >= 16383)
            k = 0;
        else
            k++;
        j++;
    }
    fclose(fp);
}

int DPDcomm::SPI_write(lms_device_t *lmsControl, uint16_t address, uint16_t data)
{

    // LLLLLLLLHL
    // 0b00000000010 00000
    uint16_t maddress = 64;
    uint16_t tempa = address + maddress;
    int ret = 0;
    if (LMS_WriteFPGAReg(lmsControl, tempa, data) != 0)
        ret = -1;
    return ret;
}

int DPDcomm::SelectCHA(int val)
{

    if (val == 1)
        chA = 1;
    else
        chA = 0;

    int ret = 0;
    uint16_t maddress = 0x0041;
    uint16_t regValue = 0x0000;
    uint16_t mask = 0x0000;
    int lsb = 4;
    int msb = 6;
    uint16_t code = 0x0000;

    LMS_ReadFPGAReg(lmsControl, maddress, &regValue);
    mask = (~(~0 << (msb - lsb + 1))) << lsb;
    regValue &= ~mask;
    if (val == 1)
        code = 1; // select CHA
    else
        code = 3; // select CHB
    regValue |= (code << lsb) & mask;

    if (LMS_WriteFPGAReg(lmsControl, maddress, regValue) != 0)
        ret = -1;

    if (chA == 1)
    {
        QADPD_GAIN = gain_chA;
        QADPD_ND = ND_chA;
    }
    else
    {
        QADPD_GAIN = gain_chB;
        QADPD_ND = ND_chB;
    }
    return ret;
}

int DPDcomm::Train()
{

    int samplesCount = samplesReceived;

    ind = 0;
    ind = Prepare_train();
    //std::cout << "[INFO] train_ind=" << ind+1 << std::endl;
    if ((ind >= 3) || (ind <= -3))
        ind = 0;
    Qadpd->skiping = QADPD_N + QADPD_ND + 1;
    Qadpd->skiping += 100;

    QADPD_YPFPGA = true;
    Qadpd->updating = QADPD_UPDATE;
    //Qadpd->reset_matrix();

    int i = 6;
    double eps = 1e-3;

    int tempi, tempr;
    int am = 1 << (QADPD_AM - 1);

    while ((Qadpd->skiping >= 0) && (i < samplesCount - 6))
    {
        Qadpd->always(xp_samples[i].r, xp_samples[i].i, x_samples[i + ind].r,
                      x_samples[i + ind].i, yp_samples[i].r, yp_samples[i].i, QADPD_YPFPGA);

        i++;
    }
    int temp = 0;

    temp = Qadpd->update_coeff(range);
    return temp;
}

void DPDcomm::End()
{
    Qadpd->release_memory();
    Qadpd->finish();
}

void DPDcomm::Log()
{

    int pa = 0;
    int dcdc = 0;

    if (chA == 1)
    {
        std::cout << "[INFO] Channel A:" << std::endl;
        pa = isOnPAchA() & 0x01;
        dcdc = isOnDCDCchA() & 0x01;
    }
    else
    {
        std::cout << "[INFO] Channel B:" << std::endl;
        pa = isOnPAchB() & 0x01;
        dcdc = isOnDCDCchB() & 0x01;
    }
    std::cout << "[INFO] DCDC=" << dcdc << " PA=" << pa << std::endl;
    std::cout << "[INFO] QADPD_N= " << QADPD_N << std::endl;
    std::cout << "[INFO] QADPD_M= " << QADPD_M << std::endl;
    std::cout << "[INFO] QADPD_ND= " << QADPD_ND << std::endl;
    std::cout << "[INFO] QADPD_GAIN= " << QADPD_GAIN << std::endl;
    std::cout << "[INFO] QADPD_LAMBDA= " << QADPD_LAMBDA << std::endl
              << ">";
}

void DPDcomm::Start()
{
    long temp = 0;
    double tempd = 0.0;
    CreateArrays();
    Qadpd->init(QADPD_N, QADPD_M, QADPD_ND, QADPD_GAIN, QADPD_LAMBDA, (1 << (QADPD_AM - 1)), QADPD_SKIP);
}

void DPDcomm::SaveDPDConfig()
{
    char buff[100];
    char buff2[100];
    iniParser m_options;
    m_options.Open(config_filename);
    m_options.SelectSection("QADPD");

    if (chA == 1)
    {
        snprintf(buff, sizeof(buff), "%d", ND_chA);
        snprintf(buff2, sizeof(buff2), "ND_chA");
        m_options.Set(buff2, buff);

        snprintf(buff, sizeof(buff), "%10.6f", gain_chA);
        snprintf(buff2, sizeof(buff2), "gain_chA");
        m_options.Set(buff2, buff);
    }
    else
    {
        snprintf(buff, sizeof(buff), "%d", ND_chB);
        snprintf(buff2, sizeof(buff2), "ND_chB");
        m_options.Set(buff2, buff);

        snprintf(buff, sizeof(buff), "%10.6f", gain_chB);
        snprintf(buff2, sizeof(buff2), "gain_chB");
        m_options.Set(buff2, buff);
    }

    snprintf(buff, sizeof(buff), "%10.6f", QADPD_LAMBDA);
    snprintf(buff2, sizeof(buff2), "QADPD_LAMBDA");
    m_options.Set(buff2, buff);

    snprintf(buff, sizeof(buff), "%d", QADPD_M);
    snprintf(buff2, sizeof(buff2), "QADPD_M");
    m_options.Set(buff2, buff);

    m_options.Save(config_filename);
}

void DPDcomm::ReadDPDConfig()
{

    int tempi = 0;
    double tempd = 0.0;
    char buff2[100];
    std::string temps2;
    iniParser m_options;
    m_options.Open(config_filename);
    m_options.SelectSection("QADPD");

    if (chA == 1)
    {
        snprintf(buff2, sizeof(buff2), "ND_chA");
        temps2 = m_options.Get(buff2, "80");
        tempi = ::atoi(temps2.c_str());
        ND_chA = tempi;
        std::cout << "[INFO] ND_chA=" << tempi << std::endl;

        snprintf(buff2, sizeof(buff2), "gain_chA");
        temps2 = m_options.Get(buff2, "1.0000");
        tempd = ::atof(temps2.c_str());
        gain_chA = tempd;
        std::cout << "[INFO] gain_chA=" << tempd << std::endl;
    }
    else
    {
        snprintf(buff2, sizeof(buff2), "ND_chB");
        temps2 = m_options.Get(buff2, "80");
        tempi = ::atoi(temps2.c_str());
        ND_chB = tempi;
        std::cout << "[INFO] ND_chB=" << tempi << std::endl;

        snprintf(buff2, sizeof(buff2), "gain_chB");
        temps2 = m_options.Get(buff2, "1.0000");
        tempd = ::atof(temps2.c_str());
        gain_chB = tempd;
        std::cout << "[INFO] gain_chB=" << tempd << std::endl;
    }

    snprintf(buff2, sizeof(buff2), "QADPD_LAMBDA");
    temps2 = m_options.Get(buff2, "0.9998");
    tempd = ::atof(temps2.c_str());
    QADPD_LAMBDA = tempd;
    std::cout << "[INFO] QADPD_LAMBDA=" << tempd << std::endl;

    snprintf(buff2, sizeof(buff2), "QADPD_M");
    temps2 = m_options.Get(buff2, "2");
    tempi = ::atoi(temps2.c_str());
    QADPD_M = tempi;
    std::cout << "[INFO] QADPD_M=" << tempi << std::endl;
}

void DPDcomm::SaveCoeff()
{

    int i = 0;
    int j = 0;

    char buff[100];
    char buff2[100];
    std::string temps;

    iniParser m_options;
    m_options.Open(config_filename);
    m_options.SelectSection("QADPD");

    for (i = 0; i <= 4; i++)
    {
        for (j = 0; j <= 3; j++) //
        {
            if (chA == 1)
            {
                snprintf(buff, sizeof(buff), "%10.6f", coeffa_chA[i][j]);
                snprintf(buff2, sizeof(buff2), "coeffa_chA_%0d_%0d", i, j);
                m_options.Set(buff2, buff);
                snprintf(buff, sizeof(buff), "%10.6f", coeffb_chA[i][j]);
                snprintf(buff2, sizeof(buff2), "coeffb_chA_%0d_%0d", i, j);
                m_options.Set(buff2, buff);
            }
            else
            {
                snprintf(buff, sizeof(buff), "%10.6f", coeffa_chB[i][j]);
                snprintf(buff2, sizeof(buff2), "coeffa_chB_%0d_%0d", i, j);
                m_options.Set(buff2, buff);
                snprintf(buff, sizeof(buff), "%10.6f", coeffb_chB[i][j]);
                snprintf(buff2, sizeof(buff2), "coeffb_chB_%0d_%0d", i, j);
                m_options.Set(buff2, buff);
            }
        }
    }
    m_options.Save(config_filename);
}

void DPDcomm::ReadCoeff()
{

    int i = 0;
    int j = 0;

    char buff2[100];
    std::string temps, temps2;
    iniParser m_options;

    double tempd;
    m_options.Open(config_filename);
    m_options.SelectSection("QADPD");

    for (i = 0; i <= 4; i++)
    {
        for (j = 0; j <= 3; j++) //
        {
            if (chA == 1)
            {
                snprintf(buff2, sizeof(buff2), "coeffa_chA_%0d_%0d", i, j);
                temps2 = m_options.Get(buff2, "0.000");
                tempd = ::atof(temps2.c_str());
                coeffa_chA[i][j] = tempd;
                //std::cout << "[INFO] coeffa_chA[" <<i << "][" << j <<"]="<< tempd <<std::endl;

                snprintf(buff2, sizeof(buff2), "coeffb_chA_%0d_%0d", i, j);
                temps2 = m_options.Get(buff2, "0.000");
                tempd = ::atof(temps2.c_str());
                coeffb_chA[i][j] = tempd;
                //std::cout << "[INFO] coeffb_chA[" <<i << "][" << j <<"]="<< tempd <<std::endl;
            }
            else
            {
                snprintf(buff2, sizeof(buff2), "coeffa_chB_%0d_%0d", i, j);
                temps2 = m_options.Get(buff2, "0.000");
                tempd = ::atof(temps2.c_str());
                coeffa_chB[i][j] = tempd;
                //std::cout << "[INFO] coeffa_chB[" <<i << "][" << j <<"]="<< tempd <<std::endl;

                snprintf(buff2, sizeof(buff2), "coeffb_chB_%0d_%0d", i, j);
                temps2 = m_options.Get(buff2, "0.000");
                tempd = ::atof(temps2.c_str());
                coeffb_chB[i][j] = tempd;
                //std::cout << "[INFO] coeffb_chB[" <<i << "][" << j <<"]="<< tempd <<std::endl;
            }
        }
    }
}
