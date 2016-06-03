#include "pllTest.h"
#include "LMS7002M.h"
#include "IConnection.h"
#include "ConnectionRegistry.h"
#include <iostream>
#include <iomanip>
#include "ErrorReporting.h"

using namespace std;
namespace lime
{

int SweepCGEN_PLL(LMS7002M* lmsControl, const double beginFreq, const double stepFreq, const double endFreq, const uint32_t tryCount, SweepCallback callback)
{
    bool stop = false;
    int testIndex = 0;
    int totalTests = (endFreq-beginFreq)/stepFreq;

    double currentFreq = beginFreq;
    while(currentFreq <= endFreq)
    {
        bool testOk = true;
        stringstream ss;
        TestResult result;
        result.successCount = 0;
        result.failureCount = 0;
        int status = 0;

        static LMS7002M::CGEN_details details;
        if(status = lmsControl->SetFrequencyCGEN(currentFreq, false, &details) != 0);
            //testOk = false;
        for(int t = 0; t < tryCount && testOk; ++t)
        {
            status = lmsControl->TuneVCO(LMS7002M::VCO_CGEN);
            //status = lmsControl->SetFrequencyCGEN(currentFreq, false);

            if(status != 0)
            {
                ++result.failureCount;
                //ss << GetLastErrorMessage();
            }
            else
                ++result.successCount;
        }

        if(result.failureCount > 0)
            testOk = false;

        result.data = &details;

        currentFreq += stepFreq;

        if(callback)
            stop = callback(testIndex, totalTests, &result);
        ++testIndex;
        if(stop)
            return ReportError(EAGAIN, "SweepCGEN_PLL: aborted by user");
    }
    return 0;
}

int SweepSX_PLL(LMS7002M* lmsControl, const double beginFreq, const double stepFreq, const double endFreq, const uint32_t tryCount, bool tx, SweepCallback callback)
{
    bool stop = false;
    int testIndex = 0;
    int totalTests = (endFreq-beginFreq)/stepFreq;

    double currentFreq = beginFreq;
    while(currentFreq <= endFreq)
    {
        bool testOk = true;
        stringstream ss;
        TestResult result;
        result.successCount = 0;
        result.failureCount = 0;
        int status = 0;

        static LMS7002M::SX_details details;
        if(status = lmsControl->SetFrequencySX(tx, currentFreq, &details) != 0);
            //testOk = false;
        
        for(int t = 0; t < tryCount && testOk; ++t)
        {
            status = lmsControl->TuneVCO(tx ? LMS7002M::VCO_SXT : LMS7002M::VCO_SXR);
            //status = lmsControl->SetFrequencyCGEN(currentFreq, false);

            if(status != 0)
            {
                ++result.failureCount;
                //ss << GetLastErrorMessage();
            }
            else
                ++result.successCount;
        }

        if(result.failureCount > 0)
            testOk = false;

        result.data = &details;

        currentFreq += stepFreq;

        if(callback)
            stop = callback(testIndex, totalTests, &result);
        ++testIndex;
        if(stop)
            return ReportError(EAGAIN, "SweepCGEN_PLL: aborted by user");
    }
    return 0;
}

}
