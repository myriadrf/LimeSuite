/* --------------------------------------------------------------------------------------------
FILE:		CFRcomm.h
DESCRIPTION     Crest Factor Reduction API
CONTENT:
AUTHOR:		Lime Microsystems LTD
DATE:		Jan 01, 2018
-------------------------------------------------------------------------------------------- */
using namespace std;
//#include <wx/string.h>
#include <string>
#include "./../API/lms7_device.h"

class CFRcomm {
public:

    // Constructors
    CFRcomm();
    ~CFRcomm();

    void Connect(lms_device_t * device);
    void SelectCHA(int val); // val=1, channel A selected
    void BypassCFR(int val); // val=1, bypass CFR
    void BypassCFRGain(int val); // val=1, bypass  Gain CFR
    void SetCFRFilterOrder(int val, int interpolation);
    void SetCFRThreshold(double threshold);
    void SetCFRGain(double threshold);

protected:

    int chA;
    struct Register {
        Register();
        Register(unsigned short address, unsigned char msb, unsigned char lsb, unsigned short defaultValue, unsigned short twocomplement);
        unsigned short address;
        unsigned char msb;
        unsigned char lsb;
        unsigned short defaultValue;
        unsigned short twocomplement; // added
    };

    lms_device_t *lmsControl;
    int SPI_write(lms_device_t * lmsControl, uint16_t address, uint16_t data);
    void SetRegValue(Register reg, uint16_t newValue);
    void UpdateHannCoeff(lms_device_t * lmsControl, uint16_t Filt_N, int chAA, int interpolation);

};
