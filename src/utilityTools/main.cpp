#include "LMS7002M.h"
#include "IConnection.h"
#include "ConnectionRegistry.h"
#include <iostream>
#include <iomanip>
#include "ErrorReporting.h"
#include "pllTest.h"
#include <getopt.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <chrono>

//#define DRAW_GNUPLOT

#ifdef DRAW_GNUPLOT
#include "gnuPlotPipe.h"
#endif // DRAW_GNUPLOT

using namespace std;
using namespace lime;

ofstream fout;

const static char* fileHeaderCGEN = "#Frequency MHz,\tok,\tfails,\tCSW,\tVCO,\tINT,\tFRAC,\tDIV_OUTCH";
const static char* fileHeaderSX =   "#Frequency MHz,\tok,\tfails,\tCSW,\tVCO,\tINT,\tFRAC,\tDIV_LOCH\tVCO_sel\tDIV2";

int linesPrinted = 0;

auto t1 = chrono::high_resolution_clock::now();
auto t2 = chrono::high_resolution_clock::now();

int stepCallbackCGEN(int testIndex, int testsCount, const TestResult* result)
{
    stringstream ss;
    if(linesPrinted++ % 10 == 0)
    {
        t2 = chrono::high_resolution_clock::now();
        cout << "Approx time remaining: " << (testsCount-testIndex)*((chrono::duration_cast<chrono::milliseconds>(t2-t1).count()/1000.0)/float(testIndex+1)) << " s" << endl;
        cout << fileHeaderCGEN << "\tRefClk" << endl;
        fout.flush();
    }
    if(result)
    {
        LMS7002M::CGEN_details *details = static_cast<LMS7002M::CGEN_details*>(result->data);
        ss << details->frequency/1e6 << "\t" << result->successCount << "\t" << result->failureCount << "\t" << details->csw << "\t" << details->frequencyVCO/1e6
        << "\t" << details->INT << "\t" << details->FRAC << "\t" << int(details->div_outch_cgen);
        printf("[%3i/%3i] ", testIndex, testsCount);
        cout << ss.str() << "\t\t" << details->referenceClock/1e6  << " " << (details->success ? "k" : "f") << endl;
        fout << ss.str() << endl;
    }
    return 0;
}

int stepCallbackSX(int testIndex, int testsCount, const TestResult* result)
{
    stringstream ss;
    if(linesPrinted++ % 10 == 0)
    {
        t2 = chrono::high_resolution_clock::now();
        cout << "Approx time remaining: " << (testsCount-testIndex)*((chrono::duration_cast<chrono::milliseconds>(t2-t1).count()/1000.0)/float(testIndex+1)) << " s" << endl;
        cout << fileHeaderSX << "\tRefClk" << endl;
        fout.flush();
    }
    if(result)
    {
        LMS7002M::SX_details *details = static_cast<LMS7002M::SX_details*>(result->data);
        ss << details->frequency/1e6 << "\t" << result->successCount << "\t" << result->failureCount << "\t" << details->csw << "\t" << details->frequencyVCO/1e6
        << "\t" << details->INT << "\t" << details->FRAC << "\t" << int(details->div_loch) << "\t" << details->sel_vco << "\t" << details->en_div2_divprog;
        printf("[%3i/%3i] ", testIndex, testsCount);
        cout << ss.str() << "\t\t" << details->referenceClock/1e6 << " " << (details->success ? "k" : "f") << endl;
        fout << ss.str() << endl;
    }
    return 0;
}

int main(int argc, char** argv)
{
    //double refClk = 30.72e6;
    double refClk = 30.72e6;
    double beginFreq = 950e6;
    double endFreq = 2175e6;
    double stepFreq = 5e6;
    uint32_t tryCount = 10;
    const char* pllNames[] = {"CGEN", "SXR", "SXT"};
    uint32_t pllSelection = 1;
    string outputFilename = "cgen_test.txt";
    string configFilename = "";
    int deviceIndex = -1;

    int c;
    while (1)
    {
        static struct option long_options[] =
        {
            {"refClk",      required_argument, 0, 'r'},
            {"beginFreq",   required_argument, 0, 'b'},
            {"stepFreq",    required_argument, 0, 's'},
            {"endFreq",     required_argument, 0, 'e'},
            {"pll",         required_argument, 0, 'p'},
            {"output",      required_argument, 0, 'o'},
            {"tryCount",    required_argument, 0, 't'},
            {"config",      required_argument, 0, 'c'},
            {"device",      required_argument, 0, 'd'},
            {"help",        no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long (argc, argv, "r:b:s:e:p:o:t:c:d:h", long_options, &option_index);

        if (c == -1) //no parameters given
            break;
        switch (c)
        {
        case 'r':{
            stringstream ss;
            ss << optarg;
            ss >> refClk;
            break;
        }
        case 'b':{
            stringstream ss;
            ss << optarg;
            ss >> beginFreq;
            break;
        }
        case 's':{
            stringstream ss;
            ss << optarg;
            ss >> stepFreq;
            break;
        }
        case 'e':{
            stringstream ss;
            ss << optarg;
            ss >> endFreq;
            break;
        }
        case 'p':{
            pllSelection = 0;
            string name(optarg);
            transform(name.begin(), name.end(), name.begin(), ::toupper);
            for(int i=0; i<3; ++i)
            {
                if(name == pllNames[i])
                {
                    pllSelection = i;
                    break;
                }
            }
            break;
        }
        case 'o':
            outputFilename = string(optarg);
            break;
        case 't':{
            stringstream ss;
            ss << optarg;
            ss >> tryCount;
            break;
        }
        case 'c':{
            stringstream ss;
            ss << optarg;
            ss >> configFilename;
            break;
        }
        case 'd':{
            stringstream ss;
            ss << optarg;
            ss >> deviceIndex;
            break;
        }
        case 'h':
            printf("help\n");
            break;
        case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            abort();
        }
    }

    cout << "Running tests for " << pllNames[pllSelection] << endl;
    cout << "beginFreq = " << beginFreq/1e6 << " MHz" << endl;
    cout << "stepFreq  = " << stepFreq/1e6 << " MHz" << endl;
    cout << "endFreq   = " << endFreq/1e6 << " MHz" << endl;
    cout << "refClk    = " << refClk/1e6 << " MHz" << endl;
    cout << "outputFilename = " << outputFilename << endl << endl;

    IConnection* serPort;
    std::vector<lime::ConnectionHandle> cachedHandles = ConnectionRegistry::findConnections();
    if(cachedHandles.size() == 0)
    {
        cout << "No devices found" << endl;
        return -1;
    }
    if(cachedHandles.size() == 1) //open the only available device
        serPort = ConnectionRegistry::makeConnection(cachedHandles.at(0));
    else //display device selection
    {
        if(deviceIndex < 0)
        {
            cout << "Device list:" << endl;
            for (size_t i = 0; i < cachedHandles.size(); i++)
               cout << setw(2) << i << ". " << cachedHandles[i].name << endl;
            cout << "Select device index (0-" << cachedHandles.size()-1 << "): ";
            int selection = 0; cin >> selection;
            selection = selection % cachedHandles.size();
            serPort = ConnectionRegistry::makeConnection(cachedHandles.at(selection));
        }
        else
            serPort = ConnectionRegistry::makeConnection(cachedHandles.at(deviceIndex));
    }
    if(serPort == nullptr)
    {
        cout << "Failed to connected to device" << endl;
        return -1;
    }
    DeviceInfo info = serPort->GetDeviceInfo();
    cout << "\nConnected to: " << info.deviceName
    << " FW: " << info.firmwareVersion << " HW: " << info.hardwareVersion << endl;

    LMS7002M* lmsControl = new LMS7002M();

    lmsControl->SetConnection(serPort, 0);
    lmsControl->ResetChip();
    if(configFilename.length() > 0)
    {
        if(lmsControl->LoadConfig(configFilename.c_str()) != 0)
        {
            cout << GetLastErrorMessage() << endl;
            return -1;
        }
    }
    else
        lmsControl->UploadAll(); //upload software defaults to chip
    serPort->SetReferenceClockRate(refClk);

#ifdef DRAW_GNUPLOT
    GNUPlotPipe gp(true);
    gp.write("set terminal X11\n");
    gp.writef("set xrange[%g:%g]\n", beginFreq/1e6, endFreq/1e6);
    gp.writef("set yrange[-1:%i]\n", tryCount+1);
    gp.write("set xlabel 'Frequency, MHz'\n");
    gp.write("set ylabel 'Failure count'\n");
    gp.write("set key autotitle columnhead\n");
#endif // DRAW_GNUPLOT
    if(pllSelection == 0)
    {
        fout.open(outputFilename.c_str(), ios::out);
        fout << fileHeaderCGEN << endl;
        SweepCallback callback = std::bind(stepCallbackCGEN, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        lime::SweepCGEN_PLL(lmsControl, beginFreq, stepFreq, endFreq, tryCount, callback);
        fout.close();
    }
    else if(pllSelection == 1)
    {
        fout.open(outputFilename.c_str(), ios::out);
        fout << fileHeaderSX << endl;
        lmsControl->SetActiveChannel(LMS7002M::ChA);
        SweepCallback callback = std::bind(stepCallbackSX, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        lime::SweepSX_PLL(lmsControl, beginFreq, stepFreq, endFreq, tryCount, false, callback);
        fout.close();
    }
    else if(pllSelection == 2)
    {
        fout.open(outputFilename.c_str(), ios::out);
        fout << fileHeaderSX << endl;
        lmsControl->SetActiveChannel(LMS7002M::ChB);
        SweepCallback callback = std::bind(stepCallbackSX, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        lime::SweepSX_PLL(lmsControl, beginFreq, stepFreq, endFreq, tryCount, true, callback);
        fout.close();
    }

    #ifdef DRAW_GNUPLOT
        gp.writef("plot \"%s\" u 1:2 with lines\n", outputFilename.c_str());
    #endif // DRAW_GNUPLOT

    ConnectionRegistry::freeConnection(serPort);
    return 0;
}
