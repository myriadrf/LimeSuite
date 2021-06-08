/* --------------------------------------------------------------------------------------------
FILE:		DPDcomm.h
DESCRIPTION     Quadrature Adaptive Digital Predistorter API
CONTENT:
AUTHOR:		Lime Microsystems LTD
DATE:		Jan 01, 2018
-------------------------------------------------------------------------------------------- */
using namespace std;
#include <string>
#include "./../API/lms7_device.h"
#include "qadpd.h"
#include "./../../external/kissFFT/kiss_fft.h"

class DPDcomm
{
public:
    // Constructors
    DPDcomm();
    ~DPDcomm();
    qadpd *Qadpd;
    void Connect(lms_device_t *device);
    void Init(int N, int M);
    void Start();
    void End();
    int ReadDataQPCIe();
    void CalibrateGain();
    void CalibrateND();
    void RunQADPD();
    void ResetCoef();
    void SendCoef();
    void UpdateCoef();
    void SaveWfm();
    int SelectCHA(int val); // val=1 for Ch.A
    double CalculateInputPower();
    void CalculateErrorPower();
    void CalculateXError(int log1);

    int IsSelectedCHA()
    {
        return chA;
    }; // val=1 for Ch.A
    int Train();
    void Log();
    void SaveCoeff();
    void ReadCoeff();
    void SaveDPDConfig();
    void ReadDPDConfig();
    void SetLambda(double val);
    void SetND(int val);
    void SetGain(double val);

    double GetLambda()
    {
        return QADPD_LAMBDA;
    };
    int GetND()
    {
        return QADPD_ND;
    };

    double GetGain()
    {
        return QADPD_GAIN;
    };
    void PAchA(int val);
    void PAchB(int val);
    void DCDCchA(int val);
    void DCDCchB(int val);
    void DPDchA(int val)
    {
        m_iDPDchA = val;
    };
    void DPDchB(int val)
    {
        m_iDPDchB = val;
    };
    int getDPDchA()
    {
        return m_iDPDchA;
    };
    int getDPDchB()
    {
        return m_iDPDchB;
    };

    int isOnPAchA();
    int isOnPAchB();
    int isOnDCDCchA();
    int isOnDCDCchB();

    void SetM(int val)
    {
        QADPD_M = val;
    };
    int GetM()
    {
        return QADPD_M;
    };

protected:
    int QADPD_N;
    int QADPD_M;
    int QADPD_ND;
    int QADPD_AM;
    int QADPD_SKIP;
    double QADPD_GAIN;
    double QADPD_LAMBDA;
    bool QADPD_YPFPGA;
    int QADPD_UPDATE;
    int samplesReceived;
    double range;
    int chA;
    int ind;
    int m_iDPDchA;
    int m_iDPDchB;
    double gain_chA;
    double gain_chB;
    int ND_chA;
    int ND_chB;
    lms_device_t *lmsControl;
    kiss_fft_cpx *xp_samples;
    kiss_fft_cpx *yp_samples;
    kiss_fft_cpx *x_samples;
    kiss_fft_cpx *y_samples;
    kiss_fft_cpx *u_samples;
    kiss_fft_cpx *error_samples;

    void CreateArrays();
    double CalculateGainCorr(double p1, double p2, double g1, double g2, double p);
    double CalculateSignalPower(int Size, double *a, double *b, double Am);
    void CalculateND(int Size, double *a, double *b, int *pND);
    int Prepare_train();
    int SPI_write(lms_device_t *lmsControl, uint16_t address, uint16_t data);
    double coeffa_chA[5][4];
    double coeffb_chA[5][4];
    double coeffa_chB[5][4];
    double coeffb_chB[5][4];
};
