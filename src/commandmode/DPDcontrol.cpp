#include <cstdlib>
#include "./../lime/LimeSuite.h"
#include <iostream>
#include <sstream>
#include <string>
#include "math.h"
#include <thread>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#define INTERVAL 3

#ifdef USE_GNU_PLOT
#include "gnuPlotPipe.h"
#endif

#include "DPDcomm.h"
#include "CFRcomm.h"

using namespace std;

//Device structure, should be initialize to NULL
lms_device_t *device = NULL;
DPDcomm *dpd = NULL;

void runADPD(void);
int timerStarted = 0;

int channel = 0;

void alarm_wakeup(int i)
{
    struct itimerval tout_val;
    signal(SIGALRM, alarm_wakeup);
    if (timerStarted == 1)
        runADPD();
    //if (timerStarted==1) cout << "[INFO] Timer\n>";
    tout_val.it_interval.tv_sec = 0;
    tout_val.it_interval.tv_usec = 0;
    tout_val.it_value.tv_sec = INTERVAL;
    tout_val.it_value.tv_usec = 0;
    if (timerStarted == 1)
        setitimer(ITIMER_REAL, &tout_val, 0);
}

void exit_func(int i)
{
    signal(SIGINT, exit_func);
    printf("\nProgram stopped.\n");
    exit(0);
}

int error()
{
    //print last error message
    cout << "ERROR:" << LMS_GetLastErrorMessage();
    if (device != NULL)
        LMS_Close(device);
    cout << "Error when connecting" << endl;
    exit(-1);
}

int extractData(string str, string *pCommand, int *pChannel, int *pBypass, int *pOrder, int *pInterpolation, double *pThreshold, int *pBypassGain, double *pGain)
{
    stringstream ss;
    string temp1, temp2, temp3, temp4, temp4b, temp5, temp6, temp7;
    int ret = 0;
    int channel = 0;
    int bypass = 0;
    int order = 0;
    int interpolation = 0;
    double threshold = 1.0;
    int bypassGain = 0;
    double gain = 1.0;

    *pCommand = "";
    *pChannel = 0;
    *pBypass = 0;
    *pOrder = 0;
    *pInterpolation = 0;
    *pThreshold = 1.0;
    *pBypassGain = 0;
    *pGain = 1.0;

    /* Storing the whole string into string stream */
    ss << str;
    ss >> temp1; // "some command"

    if (temp1 == "l")
    {
        *pCommand = temp1;
        return 1;
    }

    if (!ss.eof())
        ss >> temp2;
    else
        return 0;

    if (temp2 == "all")
        channel = 0;
    else
    {
        try
        {
            channel = stoi(temp2); // Channel(int): 1 or 2
        }
        catch (std::invalid_argument &e)
        {
            return 0;
        }

        if (!((channel == 1) || (channel == 2)))
        {
            return 0;
        }
    }

    if (((temp1 == "startPA") || (temp1 == "stopPA") || (temp1 == "startDPD") || (temp1 == "stopDPD") || (temp1 == "startDCDC") || (temp1 == "stopDCDC") ||
         (temp1 == "calibrateDPD") || (temp1 == "resetDPD") || (temp1 == "loadCoeffDPD") || (temp1 == "saveCoeffDPD") || (temp1 == "readConfigDPD") || (temp1 == "loadConfigDPD") || (temp1 == "saveConfigDPD")) &&
        ((channel == 0) || (channel == 1) || (channel == 2)))
    {
        *pCommand = temp1;
        *pChannel = channel;
        return 1;
    }
    else if ((temp1 == "setGainDPD") && ((channel == 0) || (channel == 1) || (channel == 2)))
    {

        if (!ss.eof())
            ss >> temp3;
        else
            return 0;

        try
        {
            gain = stod(temp3); // Bypass(int): 0 or 1
        }
        catch (std::invalid_argument &e)
        {
            return 0;
        }

        if ((gain >= 0.333) && (gain <= 6.0))
        {
            *pCommand = temp1;
            *pChannel = channel;
            *pGain = gain;
            return 1;
        }
        else
            return 0;
    }

    else if ((temp1 == "setLambdaDPD") && ((channel == 0) || (channel == 1) || (channel == 2)))
    {

        if (!ss.eof())
            ss >> temp3;
        else
            return 0;

        try
        {
            gain = stod(temp3); // Bypass(int): 0 or 1
        }
        catch (std::invalid_argument &e)
        {
            return 0;
        }

        if ((gain >= 0.9) && (gain < 1.0))
        {
            *pCommand = temp1;
            *pChannel = channel;
            *pGain = gain;
            return 1;
        }
        else
            return 0;
    }
    else if ((temp1 == "setND_DPD") && ((channel == 0) || (channel == 1) || (channel == 2)))
    {

        if (!ss.eof())
            ss >> temp3;
        else
            return 0;

        try
        {
            bypass = stoi(temp3); // Bypass 10 - 300
        }
        catch (std::invalid_argument &e)
        {
            return 0;
        }

        if ((bypass >= 10) && (bypass <= 300))
        {
            *pCommand = temp1;
            *pChannel = channel;
            *pBypass = bypass;
            return 1;
        }
        else
            return 0;
    }

    if (!ss.eof())
        ss >> temp3;
    else
        return 0;

    try
    {
        bypass = stoi(temp3); // Bypass(int): 0 or 1
    }
    catch (std::invalid_argument &e)
    {
        return 0;
    }

    if (!ss.eof())
        ss >> temp4;
    else
        return 0;

    try
    {
        order = stoi(temp4); // FilterOrder(int): 1 to 39
    }
    catch (std::invalid_argument &e)
    {
        return 0;
    }

    if (!ss.eof())
        ss >> temp4b;
    else
        return 0;

    try
    {
        interpolation = stoi(temp4b); // Interpolation: 0 to 1
    }
    catch (std::invalid_argument &e)
    {
        return 0;
    }

    if (!ss.eof())
        ss >> temp5;
    else
        return 0;

    try
    {
        threshold = stod(temp5); // Threshold(double): 0.0 to 1.0
    }
    catch (std::invalid_argument &e)
    {
        return 0;
    }

    if (!ss.eof())
        ss >> temp6;
    else
        return 0;

    try
    {
        bypassGain = stoi(temp6); // Bypass Gain(int): 0 or 1
    }
    catch (std::invalid_argument &e)
    {
        return 0;
    }

    if (!ss.eof())
        ss >> temp7;
    else
        return 0;

    try
    {
        gain = stod(temp7); // Gain(double): 0.0 to 1.0
    }
    catch (std::invalid_argument &e)
    {
        return 0;
    }

    if ((temp1 == "setupCFR") &&
        ((channel == 0) || (channel == 1) || (channel == 2)) &&
        ((bypass == 0) || (bypass == 1)) &&
        ((order >= 1) && (order <= 40)) &&
        ((interpolation >= 0) && (interpolation <= 1)) &&
        ((threshold >= 0.0) && (threshold <= 1.0)) &&
        ((bypassGain == 0) || (bypassGain == 1)) &&
        ((gain >= 0.333) && (gain <= 3.0)))
    {
        // correct syntax
        *pCommand = temp1;
        *pChannel = channel;
        *pBypass = bypass;
        *pOrder = order;
        *pInterpolation = interpolation;
        *pThreshold = threshold;
        *pBypassGain = bypassGain;
        *pGain = gain;

        ret = 1;
    }
    return ret;
}

int log1 = 0;

int main(int argc, char *argv[])
{
    //Find devices
    int n = 0;
    int intvar = 0;
    int channel, bypass, order, bypassGain, interpolation;
    double threshold, gain = 1.0;
    std::string command, input;

    int QADPD_M = 0; //default

    if (argc == 2)
    {
        char *p = NULL;
        int arg = strtol(argv[1], &p, 10);

        if ((*p != '\0') || (errno != 0))
        {
            cout << "[ERROR] Argument QADPD_M should be an integer in range [1, 3]" << endl; 
            return 1;                                                                       
        }
        else
        {
            QADPD_M = arg;
            if ((QADPD_M < 1) || (QADPD_M > 3))
            {
                cout << "[ERROR] QADPD_M should be an integer in range [1, 3]" << endl; 
                return 1;                                                             
            }
            cout << "[INFO] QADPD_M is " << QADPD_M << endl; 
        }
    }
    else if (argc > 2)
    {
        cout << "[ERROR] QADPD_M is single argument" << endl; 
        return 1;
    }

    struct itimerval tout_val;
    tout_val.it_interval.tv_sec = 0;
    tout_val.it_interval.tv_usec = 0;
    tout_val.it_value.tv_sec = INTERVAL; /* 10 seconds timer */
    tout_val.it_value.tv_usec = 0;
    //setitimer(ITIMER_REAL, &tout_val,0);

    signal(SIGALRM, alarm_wakeup); /* set the Alarm signal capture */
    signal(SIGINT, exit_func);

    lms_info_str_t list[8];                //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        error();

    cout << "Devices found: " << n << endl; //print number of devices
    if (n < 1)
    {
        cout << "Exit." << endl; //print number of devices
        return -1;
    }

    //open the first device
    if (LMS_Open(&device, list[0], NULL))
        error();
    cout << "Device opened" << endl;

    CFRcomm *cfr = new CFRcomm;
    cfr->Connect(device);

    dpd = new DPDcomm;
    dpd->Connect(device);
    cout << "[INFO] Device connected\n>";

    dpd->SelectCHA(1);
    dpd->ReadDPDConfig();
    std::cout << "[INFO] DPD1 configuration is loaded.\n>";
    dpd->SelectCHA(0);
    dpd->ReadDPDConfig();
    std::cout << "[INFO] DPD2 configuration is loaded.\n>";

    if (QADPD_M != 0)
        dpd->SetM(QADPD_M);
    
    QADPD_M = dpd->GetM();
    if ((QADPD_M < 1) || (QADPD_M > 3))
    {
        cout << "[ERROR] QADPD_M should be an integer in range [1, 3]" << endl; //print number of devices
        return 1;                                                               // In main(), returning non-zero means failure
    }
    dpd->Init(4, QADPD_M);

    while ((getline(std::cin, input)) && (input != "quit"))
    {

        if (input == "help")
        {
            std::cout << "[INFO] The commands are: "
                      << "\n";
            std::cout << "[INFO] startPA {1|2|all}"
                      << "\n";
            std::cout << "[INFO] stopPA {1|2|all}"
                      << "\n";
            std::cout << "[INFO] startDCDC {1|2|all}"
                      << "\n";
            std::cout << "[INFO] stopDCDC {1|2|all}"
                      << "\n";
            std::cout << "[INFO] calibrateDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] resetDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] startDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] stopDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] readConfigDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] loadConfigDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] saveConfigDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] loadCoeffDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] saveCoeffDPD {1|2|all}"
                      << "\n";
            std::cout << "[INFO] setGainDPD {1|2|all} <gain[1.0..6.0]>"
                      << "\n";
            std::cout << "[INFO] setLambdaDPD {1|2|all} <lambda[0.9..1.0]>"
                      << "\n";
            std::cout << "[INFO] setND_DPD {1|2|all} <delay[10..300]>"
                      << "\n";
            std::cout << "[INFO] setupCFR {1|2|all} <bypass{0|1}> <order[1..40]> interp{0|1} <threshold[0.0..1.0]> <bypassgain{0|1}> <gain[0.33..1.33]>"
                      << "\n";
            std::cout << "[INFO] l"
                      << "\n>";
            std::cout << "[INFO] quit"
                      << "\n>";
        }
        else if (extractData(input, &command, &channel, &bypass, &order, &interpolation, &threshold, &bypassGain, &gain) == 1)
        {

            if (command == "startPA")
            {
                if (channel == 0)
                {
                    if (dpd->isOnDCDCchA())
                    {
                        dpd->PAchA(1);
                        std::cout << "[INFO] PA1 is turned on.\n";
                    }
                    else
                        std::cout << "[WARN] Turn on DCDC1 first.\n";

                    if (dpd->isOnDCDCchB())
                    {
                        dpd->PAchB(1);
                        std::cout << "[INFO] PA2 is turned on.\n>";
                    }
                    else
                        std::cout << "[WARN] Turn on DCDC2 first.\n>";
                }
                else if (channel == 1)
                {
                    if (dpd->isOnDCDCchA())
                    {
                        dpd->PAchA(1);
                        std::cout << "[INFO] PA" << channel << " is turned on.\n>";
                    }
                    else
                        std::cout << "[WARN] Turn on DCDC" << channel << " first.\n>";
                }
                else
                {
                    if (dpd->isOnDCDCchB())
                    {
                        dpd->PAchB(1);
                        std::cout << "[INFO] PA" << channel << " is turned on.\n>";
                    }
                    else
                        std::cout << "[WARN] Turn on DCDC" << channel << " first.\n>";
                }
            }
            else if (command == "stopPA")
            {
                if (channel == 0)
                {
                    dpd->PAchA(0);
                    std::cout << "[INFO] PA1 is turned off.\n";
                    dpd->PAchB(0);
                    std::cout << "[INFO] PA2 is turned off.\n>";
                }
                else if (channel == 1)
                {
                    dpd->PAchA(0);
                    std::cout << "[INFO] PA" << channel << " is turned off.\n>";
                }
                else
                {
                    dpd->PAchB(0);
                    std::cout << "[INFO] PA" << channel << " is turned off.\n>";
                }
            }
            else if (command == "startDCDC")
            {
                if (channel == 0)
                {
                    dpd->DCDCchA(1);
                    std::cout << "[INFO] DCDC1 is turned on.\n";
                    dpd->DCDCchB(1);
                    std::cout << "[INFO] DCDC2 is turned on.\n>";
                }
                else if (channel == 1)
                {
                    dpd->DCDCchA(1);
                    std::cout << "[INFO] DCDC" << channel << " is turned on.\n>";
                }
                else
                {
                    dpd->DCDCchB(1);
                    std::cout << "[INFO] DCDC" << channel << " is turned on.\n>";
                }
            }
            else if (command == "stopDCDC")
            {
                if (channel == 0)
                {
                    dpd->PAchA(0);
                    dpd->DCDCchA(0);
                    std::cout << "[INFO] DCDC1, PA1 are turned off.\n";
                    dpd->PAchB(0);
                    dpd->DCDCchB(0);
                    std::cout << "[INFO] DCDC2, PA2 are turned off.\n>";
                }
                else if (channel == 1)
                {
                    dpd->PAchA(0);
                    dpd->DCDCchA(0);
                    std::cout << "[INFO] DCDC1, PA1 are turned off.\n>";
                }
                else
                {
                    dpd->PAchB(0);
                    dpd->DCDCchB(0);
                    std::cout << "[INFO] DCDC2, PA2 are turned off.\n>";
                }
            }
            else if (command == "startDPD")
            {
                if (channel == 0)
                {
                    dpd->DPDchA(1);
                    std::cout << "[INFO] DPD1 training is started.\n";
                    dpd->SelectCHA(1);
                    dpd->Log();

                    dpd->DPDchB(1);
                    std::cout << "[INFO] DPD2 training is started.\n>";
                    dpd->SelectCHA(0);
                    dpd->Log();
                }
                else if (channel == 1)
                {
                    dpd->DPDchA(1);
                    std::cout << "[INFO] DPD" << channel << " training is started.\n>";
                    dpd->SelectCHA(2 - channel);
                    dpd->Log();
                }
                else
                {
 
                    dpd->DPDchB(1);
                    std::cout << "[INFO] DPD" << channel << " training is started.\n>";
                    dpd->SelectCHA(2 - channel);
                    dpd->Log();
                }
                if ((timerStarted == 0) && ((dpd->getDPDchA() == 1) || (dpd->getDPDchB() == 1)))
                {
                    timerStarted = 1;
                    setitimer(ITIMER_REAL, &tout_val, 0);
                }
            }
            else if (command == "stopDPD")
            {
                if (channel == 0)
                {
                    dpd->DPDchA(0);
                    std::cout << "[INFO] DPD1 training is stopped.\n";
                    dpd->DPDchB(0);
                    std::cout << "[INFO] DPD2 training is stopped.\n>";
                }
                else if (channel == 1)
                {
                    dpd->DPDchA(0);
                    std::cout << "[INFO] DPD" << channel << " training is stopped.\n>";
                }
                else
                {
                    dpd->DPDchB(0);
                    std::cout << "[INFO] DPD" << channel << " training is stopped.\n>";
                }

                if ((timerStarted == 1) && (dpd->getDPDchA() == 0) && (dpd->getDPDchB() == 0))
                {
                    timerStarted = 0;
                }
            }
            else if (command == "readConfigDPD")
            {
                if (channel == 0)
                {
                    dpd->SelectCHA(1);
                    dpd->Log();
                    dpd->SelectCHA(0);
                    dpd->Log();
                }
                else
                {
                    dpd->SelectCHA(2 - channel);
                    dpd->Log();
                }
            }
            else if (command == "setGainDPD")
            {
                if (channel == 0)
                {
                    dpd->SelectCHA(1);
                    dpd->SetGain(gain);
                    std::cout << "[INFO] DPD1 QADPD_GAIN= " << gain << "\n";
                    dpd->SelectCHA(0);
                    dpd->SetGain(gain);
                    std::cout << "[INFO] DPD2 QADPD_GAIN= " << gain << "\n>";
                }
                else
                {
                    dpd->SelectCHA(2 - channel);
                    dpd->SetGain(gain);
                    std::cout << "[INFO] DPD" << channel << " QADPD_GAIN= " << gain << "\n>";
                }
            }
            else if (command == "setND_DPD")
            {
                if (channel == 0)
                {
                    dpd->SelectCHA(1);
                    dpd->SetND(bypass);
                    std::cout << "[INFO] DPD1 QADPD_ND= " << bypass << "\n";
                    dpd->SelectCHA(0);
                    dpd->SetND(bypass);
                    std::cout << "[INFO] DPD2 QADPD_ND= " << bypass << "\n>";
                }
                else
                {
                    dpd->SelectCHA(2 - channel);
                    dpd->SetND(bypass);
                    std::cout << "[INFO] DPD" << channel << " QADPD_ND= " << bypass << "\n>";
                }
            }
            else if (command == "setLambdaDPD")
            {
                if (channel == 0)
                {
                    dpd->SelectCHA(1);
                    dpd->SetLambda(gain);
                    std::cout << "[INFO] DPD1 QADPD_LAMBDA= " << gain << "\n";
                    dpd->SelectCHA(0);
                    dpd->SetLambda(gain);
                    std::cout << "[INFO] DPD2 QADPD_LAMBDA= " << gain << "\n>";
                }
                else
                {
                    dpd->SelectCHA(2 - channel);
                    dpd->SetLambda(gain);
                    std::cout << "[INFO] DPD" << channel << " QADPD_LAMBDA= " << gain << "\n>";
                }
            }
            else if (command == "resetDPD")
            {
                if (timerStarted == 0)
                {
                    if (channel == 0)
                    {
                        dpd->SelectCHA(1);
                        dpd->ResetCoef();
                        dpd->SendCoef();
                        std::cout << "[INFO] DPD1 is reset.\n";
                        dpd->SelectCHA(0);
                        dpd->ResetCoef();
                        dpd->SendCoef();
                        std::cout << "[INFO] DPD2 is reset.\n>";
                    }
                    else
                    {
                        dpd->SelectCHA(2 - channel);
                        dpd->ResetCoef();
                        dpd->SendCoef();
                        std::cout << "[INFO] DPD" << channel << " is reset.\n>";
                    }
                }
                else
                    std::cout << "Stop the DPD first.\n>";
            }
            else if (command == "calibrateDPD")
            {
                if (timerStarted == 0)
                {
                    if (channel == 0)
                    {
                            dpd->SelectCHA(1);
                            dpd->CalibrateND();
                            dpd->CalibrateGain();
                            std::cout << "[INFO] DPD1 calibration is done.\n";

                            dpd->SelectCHA(0);
                            dpd->CalibrateND();
                            dpd->CalibrateGain();
                            std::cout << "[INFO] DPD2 calibration is done.\n>";
                    }
                    else if (channel == 1)
                    {
                            dpd->SelectCHA(1);
                            dpd->CalibrateND();
                            dpd->CalibrateGain();
                            std::cout << "[INFO] DPD1 calibration is done.\n>";

                    }
                    else
                    {
                            dpd->SelectCHA(0);
                            dpd->CalibrateND();
                            dpd->CalibrateGain();
                            std::cout << "[INFO] DPD2 calibration is done.\n>";
                    }
                }
                else
                    std::cout << "Stop the DPDs first.\n>";
            }
            else if (command == "loadCoeffDPD")
            {
                if (channel == 0)
                {
                    dpd->SelectCHA(1);
                    dpd->ReadCoeff();
                    dpd->UpdateCoef();
                    std::cout << "[INFO] DPD1 coefficients are loaded.\n";
                    dpd->SelectCHA(0);
                    dpd->ReadCoeff();
                    dpd->UpdateCoef();
                    std::cout << "[INFO] DPD2 coefficients are loaded.\n>";
                }
                else
                {
                    dpd->SelectCHA(2 - channel);
                    dpd->ReadCoeff();
                    dpd->UpdateCoef();
                    std::cout << "[INFO] DPD" << channel << " coefficients are loaded.\n>";
                }
            }
            else if (command == "loadConfigDPD")
            {
                if (channel == 0)
                {
                    dpd->SelectCHA(1);
                    dpd->ReadDPDConfig();
                    std::cout << "[INFO] DPD1 configuration is loaded.\n";
                    dpd->SelectCHA(0);
                    dpd->ReadDPDConfig();
                    std::cout << "[INFO] DPD2 configuration is loaded.\n>";
                }
                else
                {
                    dpd->SelectCHA(2 - channel);
                    dpd->ReadDPDConfig();
                    std::cout << "[INFO] DPD" << channel << " configuration is loaded.\n>";
                }
            }
            else if (command == "saveConfigDPD")
            {
                if (channel == 0)
                {
                    dpd->SelectCHA(1);
                    dpd->SaveDPDConfig();
                    std::cout << "[INFO] DPD1 configuration is saved.\n";
                    dpd->SelectCHA(0);
                    dpd->SaveDPDConfig();
                    std::cout << "[INFO] DPD2 configuration is saved.\n>";
                }
                else
                {
                    dpd->SelectCHA(2 - channel);
                    dpd->SaveDPDConfig();
                    std::cout << "[INFO] DPD" << channel << " configuration is saved.\n>";
                }
            }
            else if (command == "saveCoeffDPD")
            {
                if (channel == 0)
                {
                    dpd->SelectCHA(1);
                    dpd->SaveCoeff();
                    std::cout << "[INFO] DPD1 coefficients are saved.\n";
                    dpd->SelectCHA(0);
                    dpd->SaveCoeff();
                    std::cout << "[INFO] DPD2 coefficients are saved.\n>";
                }
                else
                {
                    dpd->SelectCHA(2 - channel);
                    dpd->SaveCoeff();
                    std::cout << "[INFO] DPD" << channel << " coefficients are saved.\n>";
                }
            }
            else if (command == "l")
            {
                log1 = 1 - log1;
                std::cout << "\n[INFO] log " << log1 << " \n>";
            }
            else if (command == "setupCFR")
            {
                if (timerStarted == 0)
                {
                    if (channel == 0)
                    {
                        cfr->SelectCHA(1);      // select channnel
                        cfr->BypassCFR(bypass); // if 1, CFR is bypassed
                        cfr->SetCFRFilterOrder(order, interpolation);
                        cfr->SetCFRThreshold(threshold);
                        cfr->BypassCFRGain(bypassGain); // if 1, Gain is bypassed
                        cfr->SetCFRGain(gain);
                        std::cout << "[INFO] CFR1 setup is done.\n";
                        cfr->SelectCHA(0);      // select channnel
                        cfr->BypassCFR(bypass); // if 1, CFR is bypassed
                        cfr->SetCFRFilterOrder(order, interpolation);
                        cfr->SetCFRThreshold(threshold);
                        cfr->BypassCFRGain(bypassGain); // if 1, Gain is bypassed
                        cfr->SetCFRGain(gain);
                        std::cout << "[INFO] CFR2 setup is done.\n>";
                    }
                    else
                    {
                        cfr->SelectCHA(2 - channel); // select channnel
                        cfr->BypassCFR(bypass);      // if 1, CFR is bypassed
                        cfr->SetCFRFilterOrder(order, interpolation);
                        cfr->SetCFRThreshold(threshold);
                        cfr->BypassCFRGain(bypassGain); // if 1, Gain is bypassed
                        cfr->SetCFRGain(gain);
                        std::cout << "[INFO] CFR" << channel << " setup is done.\n>";
                    }
                }
                else
                    std::cout << "[INFO] Stop the DPD first.\n>";
            }
        }
        else
        {
            std::cout << "[INFO] Unrecognized command: " << input << "!\n>";
        }
    }
    LMS_Close(device);
    return 0;
}

void runADPD(void)
{
    //std::cout << "[INFO] DPD run." << "\n>";
    if (channel == 0)
        channel = 1;
    else
        channel = 0;

    if ((channel == 0) && (dpd->getDPDchA() == 1))
    {
        dpd->SelectCHA(1);
        dpd->Start();
        for (int i = 0; i <= 0; i++)
        {
            dpd->ReadDataQPCIe();

            if (dpd->CalculateInputPower() >= 0.03)
            {
                dpd->Train();
                dpd->SendCoef();
                dpd->RunQADPD();
                if (log1)
                    std::cout << "[INFO] Channel A: one training process is done. \n";
            }
        }
        dpd->CalculateXError(log1);
        dpd->End();
    }

    if ((channel == 1) && (dpd->getDPDchB() == 1))
    {
        dpd->SelectCHA(0);
        dpd->Start();
        for (int i = 0; i <= 0; i++)
        {
            dpd->ReadDataQPCIe();

            if (dpd->CalculateInputPower() >= 0.03)
            {
                dpd->Train();
                dpd->SendCoef();
                dpd->RunQADPD();
                if (log1)
                    std::cout << "[INFO] Channel B: one training process is done. \n";
            }
        }
        dpd->CalculateXError(log1);
        dpd->End();
    }
}
