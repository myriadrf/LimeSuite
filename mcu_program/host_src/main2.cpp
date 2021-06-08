#include <stdio.h>
#include <fstream>
#include "mcuHexBin.h"
using namespace std;


static bool useMCU =1;

int main(int argc, char** argv)
{

 int status = 0;

 if(useMCU)
    {
        uint8_t mcuImage[8192*2];
        uint16_t imgSize = 0;
        status = MCU_HEX2BIN("calibrationsLMS7_MCU.hex", sizeof(mcuImage), mcuImage, &imgSize);
        if(status != 0)
            return status;

        fstream fout;
        fout.open("mcu_dc_iq_calibration.cpp", ios::out);

        //fout << "#include mcu_programs.h"""<<endl<<endl;
        fout << "const uint8_t mcu_program_lms7_dc_iq_calibration_bin[16384] = {";
        for(size_t i = 0; i < sizeof(mcuImage); ++i)
        {
            if(i % 16 == 0)
                fout << endl;
            char ctemp[40];
            sprintf(ctemp, "0x%02X,", mcuImage[i]);
            fout << ctemp;
        }
        fout << "};\n";
        fout.close();

        //status = MCU_UploadProgram(mcuImage, sizeof(mcuImage));
        //if(status != 0)
        //{
        //    printf("MCU programming failed\n");
        //    return -1;
        //}
    }
  

}
