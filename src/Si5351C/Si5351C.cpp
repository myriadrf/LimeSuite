/**
@file	Si5351C.cpp
@author	Lime Microsystems
@brief 	Configuring of Si5351C
*/

#include "Si5351C.h"
#include "Logger.h"
#include <math.h>
#include <iomanip>
#include <fstream>
#include <set>
#include <map>
#include <cstring>
#include <cassert>
#include <ciso646> // alternative operators for visual c++: not, and, or...
#include <vector>
#include <cstddef>
#include "limesuite/IComms.h"

using namespace std;
using namespace lime;

// TODO: check if actually needed
// static uint8_t addrSi5351 = 0x20;

/// Splits float into fraction integers A + B/C
void realToFrac(const float real, int& A, int& B, int& C)
{
    A = (int)real;
    B = (int)((real - A) * 1048576 + 0.5);
    C = 1048576;

    int a = B;
    int b = C;
    int temp;
    while (b != 0) // greatest common divider
    {
        temp = a % b;
        a = b;
        b = temp;
    }
    B = B / a;
    C = C / a;
}

/// Default configuration
const std::vector<Si5351C::Si5351CConfigurationValue> Si5351C::m_defaultConfiguration = { {
    { 0, std::byte{ 0x00 } },
    { 1, std::byte{ 0x00 } },
    { 2, std::byte{ 0x08 } },
    { 3, std::byte{ 0x00 } },
    { 4, std::byte{ 0x00 } },
    { 5, std::byte{ 0x00 } },
    { 6, std::byte{ 0x00 } },
    { 7, std::byte{ 0x00 } },
    { 8, std::byte{ 0x00 } },
    { 9, std::byte{ 0x00 } },
    { 10, std::byte{ 0x00 } },
    { 11, std::byte{ 0x00 } },
    { 12, std::byte{ 0x00 } },
    { 13, std::byte{ 0x00 } },
    { 14, std::byte{ 0x00 } },
    { 15, std::byte{ 0x04 } },
    { 16, std::byte{ 0x4F } },
    { 17, std::byte{ 0x4F } },
    { 18, std::byte{ 0x4F } },
    { 19, std::byte{ 0x4F } },
    { 20, std::byte{ 0x4F } },
    { 21, std::byte{ 0x4F } },
    { 22, std::byte{ 0x0F } },
    { 23, std::byte{ 0x0F } },
    { 24, std::byte{ 0x00 } },
    { 25, std::byte{ 0x00 } },
    { 26, std::byte{ 0x00 } },
    { 27, std::byte{ 0x80 } },
    { 28, std::byte{ 0x00 } },
    { 29, std::byte{ 0x0B } },
    { 30, std::byte{ 0x2F } },
    { 31, std::byte{ 0x00 } },
    { 32, std::byte{ 0x00 } },
    { 33, std::byte{ 0x00 } },
    { 34, std::byte{ 0x00 } },
    { 35, std::byte{ 0x00 } },
    { 36, std::byte{ 0x00 } },
    { 37, std::byte{ 0x00 } },
    { 38, std::byte{ 0x00 } },
    { 39, std::byte{ 0x00 } },
    { 40, std::byte{ 0x00 } },
    { 41, std::byte{ 0x00 } },
    { 42, std::byte{ 0x00 } },
    { 43, std::byte{ 0x01 } },
    { 44, std::byte{ 0x00 } },
    { 45, std::byte{ 0x0D } },
    { 46, std::byte{ 0x00 } },
    { 47, std::byte{ 0x00 } },
    { 48, std::byte{ 0x00 } },
    { 49, std::byte{ 0x00 } },
    { 50, std::byte{ 0x00 } },
    { 51, std::byte{ 0x01 } },
    { 52, std::byte{ 0x00 } },
    { 53, std::byte{ 0x0D } },
    { 54, std::byte{ 0x00 } },
    { 55, std::byte{ 0x00 } },
    { 56, std::byte{ 0x00 } },
    { 57, std::byte{ 0x00 } },
    { 58, std::byte{ 0x00 } },
    { 59, std::byte{ 0x01 } },
    { 60, std::byte{ 0x00 } },
    { 61, std::byte{ 0x0D } },
    { 62, std::byte{ 0x00 } },
    { 63, std::byte{ 0x00 } },
    { 64, std::byte{ 0x00 } },
    { 65, std::byte{ 0x00 } },
    { 66, std::byte{ 0x00 } },
    { 67, std::byte{ 0x01 } },
    { 68, std::byte{ 0x00 } },
    { 69, std::byte{ 0x0D } },
    { 70, std::byte{ 0x00 } },
    { 71, std::byte{ 0x00 } },
    { 72, std::byte{ 0x00 } },
    { 73, std::byte{ 0x00 } },
    { 74, std::byte{ 0x00 } },
    { 75, std::byte{ 0x01 } },
    { 76, std::byte{ 0x00 } },
    { 77, std::byte{ 0x0D } },
    { 78, std::byte{ 0x00 } },
    { 79, std::byte{ 0x00 } },
    { 80, std::byte{ 0x00 } },
    { 81, std::byte{ 0x00 } },
    { 82, std::byte{ 0x00 } },
    { 83, std::byte{ 0x01 } },
    { 84, std::byte{ 0x00 } },
    { 85, std::byte{ 0x0D } },
    { 86, std::byte{ 0x00 } },
    { 87, std::byte{ 0x00 } },
    { 88, std::byte{ 0x00 } },
    { 89, std::byte{ 0x00 } },
    { 90, std::byte{ 0x1E } },
    { 91, std::byte{ 0x1E } },
    { 92, std::byte{ 0x00 } },
    { 93, std::byte{ 0x00 } },
    { 94, std::byte{ 0x00 } },
    { 95, std::byte{ 0x00 } },
    { 96, std::byte{ 0x00 } },
    { 97, std::byte{ 0x00 } },
    { 98, std::byte{ 0x00 } },
    { 99, std::byte{ 0x00 } },
    { 100, std::byte{ 0x00 } },
    { 101, std::byte{ 0x00 } },
    { 102, std::byte{ 0x00 } },
    { 103, std::byte{ 0x00 } },
    { 104, std::byte{ 0x00 } },
    { 105, std::byte{ 0x00 } },
    { 106, std::byte{ 0x00 } },
    { 107, std::byte{ 0x00 } },
    { 108, std::byte{ 0x00 } },
    { 109, std::byte{ 0x00 } },
    { 110, std::byte{ 0x00 } },
    { 111, std::byte{ 0x00 } },
    { 112, std::byte{ 0x00 } },
    { 113, std::byte{ 0x00 } },
    { 114, std::byte{ 0x00 } },
    { 115, std::byte{ 0x00 } },
    { 116, std::byte{ 0x00 } },
    { 117, std::byte{ 0x00 } },
    { 118, std::byte{ 0x00 } },
    { 119, std::byte{ 0x00 } },
    { 120, std::byte{ 0x00 } },
    { 121, std::byte{ 0x00 } },
    { 122, std::byte{ 0x00 } },
    { 123, std::byte{ 0x00 } },
    { 124, std::byte{ 0x00 } },
    { 125, std::byte{ 0x00 } },
    { 126, std::byte{ 0x00 } },
    { 127, std::byte{ 0x00 } },
    { 128, std::byte{ 0x00 } },
    { 129, std::byte{ 0x00 } },
    { 130, std::byte{ 0x00 } },
    { 131, std::byte{ 0x00 } },
    { 132, std::byte{ 0x00 } },
    { 133, std::byte{ 0x00 } },
    { 134, std::byte{ 0x00 } },
    { 135, std::byte{ 0x00 } },
    { 136, std::byte{ 0x00 } },
    { 137, std::byte{ 0x00 } },
    { 138, std::byte{ 0x00 } },
    { 139, std::byte{ 0x00 } },
    { 140, std::byte{ 0x00 } },
    { 141, std::byte{ 0x00 } },
    { 142, std::byte{ 0x00 } },
    { 143, std::byte{ 0x00 } },
    { 144, std::byte{ 0x00 } },
    { 145, std::byte{ 0x00 } },
    { 146, std::byte{ 0x00 } },
    { 147, std::byte{ 0x00 } },
    { 148, std::byte{ 0x00 } },
    { 149, std::byte{ 0x00 } },
    { 150, std::byte{ 0x00 } },
    { 151, std::byte{ 0x00 } },
    { 152, std::byte{ 0x00 } },
    { 153, std::byte{ 0x00 } },
    { 154, std::byte{ 0x00 } },
    { 155, std::byte{ 0x00 } },
    { 156, std::byte{ 0x00 } },
    { 157, std::byte{ 0x00 } },
    { 158, std::byte{ 0x00 } },
    { 159, std::byte{ 0x00 } },
    { 160, std::byte{ 0x00 } },
    { 161, std::byte{ 0x00 } },
    { 162, std::byte{ 0x00 } },
    { 163, std::byte{ 0x00 } },
    { 164, std::byte{ 0x00 } },
    { 165, std::byte{ 0x00 } },
    { 166, std::byte{ 0x00 } },
    { 167, std::byte{ 0x00 } },
    { 168, std::byte{ 0x00 } },
    { 169, std::byte{ 0x00 } },
    { 170, std::byte{ 0x00 } },
    { 171, std::byte{ 0x00 } },
    { 172, std::byte{ 0x00 } },
    { 173, std::byte{ 0x00 } },
    { 174, std::byte{ 0x00 } },
    { 175, std::byte{ 0x00 } },
    { 176, std::byte{ 0x00 } },
    { 177, std::byte{ 0x00 } },
    { 178, std::byte{ 0x00 } },
    { 179, std::byte{ 0x00 } },
    { 180, std::byte{ 0x00 } },
    { 181, std::byte{ 0x30 } },
    { 182, std::byte{ 0x00 } },
    { 183, std::byte{ 0xD2 } },
    { 184, std::byte{ 0x60 } },
    { 185, std::byte{ 0x60 } },
    { 186, std::byte{ 0x00 } },
    { 187, std::byte{ 0xC0 } },
    { 188, std::byte{ 0x00 } },
    { 189, std::byte{ 0x00 } },
    { 190, std::byte{ 0x00 } },
    { 191, std::byte{ 0x00 } },
    { 192, std::byte{ 0x00 } },
    { 193, std::byte{ 0x00 } },
    { 194, std::byte{ 0x00 } },
    { 195, std::byte{ 0x00 } },
    { 196, std::byte{ 0x00 } },
    { 197, std::byte{ 0x00 } },
    { 198, std::byte{ 0x00 } },
    { 199, std::byte{ 0x00 } },
    { 200, std::byte{ 0x00 } },
    { 201, std::byte{ 0x00 } },
    { 202, std::byte{ 0x00 } },
    { 203, std::byte{ 0x00 } },
    { 204, std::byte{ 0x00 } },
    { 205, std::byte{ 0x00 } },
    { 206, std::byte{ 0x00 } },
    { 207, std::byte{ 0x00 } },
    { 208, std::byte{ 0x00 } },
    { 209, std::byte{ 0x00 } },
    { 210, std::byte{ 0x00 } },
    { 211, std::byte{ 0x00 } },
    { 212, std::byte{ 0x00 } },
    { 213, std::byte{ 0x00 } },
    { 214, std::byte{ 0x00 } },
    { 215, std::byte{ 0x00 } },
    { 216, std::byte{ 0x00 } },
    { 217, std::byte{ 0x00 } },
    { 218, std::byte{ 0x00 } },
    { 219, std::byte{ 0x00 } },
    { 220, std::byte{ 0x00 } },
    { 221, std::byte{ 0x0D } },
    { 222, std::byte{ 0x00 } },
    { 223, std::byte{ 0x00 } },
    { 224, std::byte{ 0x00 } },
    { 225, std::byte{ 0x00 } },
    { 226, std::byte{ 0x00 } },
    { 227, std::byte{ 0x00 } },
    { 228, std::byte{ 0x00 } },
    { 229, std::byte{ 0x00 } },
    { 230, std::byte{ 0x00 } },
    { 231, std::byte{ 0x00 } },
    { 232, std::byte{ 0x00 } },
} };

// ---------------------------------------------------------------------------
Si5351C::Si5351C(lime::II2C& i2c_comms)
    : comms(i2c_comms)
{
    Reset();
}
// ---------------------------------------------------------------------------

Si5351C::~Si5351C()
{
}

/** @brief Sends Configuration to Si5351C
*/
Si5351C::Status Si5351C::UploadConfiguration()
{
    std::vector<std::byte> outBuffer;
    //Disable outputs
    outBuffer.push_back(std::byte{ 0x03 });
    outBuffer.push_back(std::byte{ 0xFF });
    //Power down all output drivers
    for (uint8_t i = 0U; i < 8U; ++i)
    {
        outBuffer.push_back(static_cast<std::byte>(16 + i));
        outBuffer.push_back(std::byte{ 0x84 });
    }
    //write new configuration
    for (uint8_t i = 15U; i <= 92U; ++i)
    {
        outBuffer.push_back(std::byte{ i });
        outBuffer.push_back(m_newConfiguration[i]);
    }
    for (uint8_t i = 149U; i <= 170U; ++i)
    {
        outBuffer.push_back(std::byte{ i });
        outBuffer.push_back(m_newConfiguration[i]);
    }
    //apply soft reset
    outBuffer.push_back(std::byte{ 177 });
    outBuffer.push_back(std::byte{ 0xAC });
    //Enabe desired outputs
    outBuffer.push_back(std::byte{ 3 });
    outBuffer.push_back(m_newConfiguration[3]);

    try
    {
        comms.I2CWrite(addrSi5351, outBuffer.data(), outBuffer.size());
        return Status::SUCCESS;
    } catch (std::runtime_error& e)
    {
        lime::error("Si5351C configuration failed %s", e.what());
        return Status::FAILED;
    }
}

/**
    @brief Loads register values for Si5356A from file
    @param FName input filename
*/
bool Si5351C::LoadRegValuesFromFile(string FName)
{
    fstream fin;
    fin.open(FName, ios::in);

    const int len = 1024;
    char line[len];

    int addr;
    unsigned int value;

    while (!fin.eof())
    {
        fin.getline(line, len);
        if (line[0] == '#')
            continue;
        if (strcmp(line, "#END_PROFILE") == 0)
            break;
        sscanf(line, "%i,%x", &addr, &value);
        m_newConfiguration[addr] = static_cast<std::byte>(value);
    }

    fin.close();
    return false;
}

/** @brief Calculates multisynth dividers and VCO frequencies
    @param clocks output clocks configuration
    @param plls plls configurations
    @param Fmin lowest VCO frequency
    @param Fmax highest VCO frequency
*/
void Si5351C::FindVCO(Si5351_Channel* clocks, Si5351_PLL* plls, const unsigned long Fmin, const unsigned long Fmax)
{
    int clockCount = 8;
    //reset output parameters
    for (int i = 0; i < clockCount; i++)
    {
        clocks[i].pllSource = 0;
        clocks[i].int_mode = 0;
        clocks[i].multisynthDivider = 8;
    }

    bool clk6satisfied = !clocks[6].powered;
    bool clk7satisfied = !clocks[7].powered;

    bool pllAused = false;
    bool pllBused = false;

    map<unsigned long, int> availableFrequenciesPLLA; //all available frequencies for VCO
    map<unsigned long, int> availableFrequenciesPLLB; //all available frequencies for VCO

    //if clk6 or clk7 is used make available frequencies according to them
    if (clocks[6].powered || clocks[7].powered)
    {
        set<unsigned long> clk6freqs;
        set<unsigned long> clk7freqs;
        set<unsigned long> sharedFreqs;
        unsigned int mult = 6;
        if (!clk6satisfied)
        {
            unsigned long freq = clocks[6].outputFreqHz;
            while (freq <= Fmax && mult <= 254)
            {
                freq = clocks[6].outputFreqHz * mult;
                if (freq >= Fmin && freq <= Fmax)
                {
                    clk6freqs.insert(freq);
                }
                mult += 2;
            }
        }
        mult = 6;
        if (!clk7satisfied)
        {
            unsigned long freq = clocks[7].outputFreqHz;
            while (freq <= Fmax && mult <= 254)
            {
                freq = clocks[7].outputFreqHz * mult;
                if (freq >= Fmin && freq <= Fmax)
                {
                    clk7freqs.insert(freq);
                }
                mult += 2;
            }
        }
        bool canShare = false;
        //find if clk6 and clk7 can share the same pll
        for (set<unsigned long>::iterator it6 = clk6freqs.begin(); it6 != clk6freqs.end(); ++it6)
        {
            for (set<unsigned long>::iterator it7 = clk7freqs.begin(); it7 != clk7freqs.end(); ++it7)
            {
                if (*it6 == *it7)
                {
                    canShare = true;
                    sharedFreqs.insert(*it6);
                }
            }
        }
        if (canShare) //assign PLLA for both clocks
        {
            clocks[6].pllSource = 0;
            clocks[7].pllSource = 0;
            pllAused = true;
            for (set<unsigned long>::iterator it = sharedFreqs.begin(); it != sharedFreqs.end(); ++it)
            {
                availableFrequenciesPLLA.insert(pair<unsigned long, int>(*it, 0));
            }
        }
        else //if clocks 6 and 7 can't share pll, assign pllA to clk6 and pllB to clk7
        {
            if (!clk6satisfied)
            {
                clocks[6].pllSource = 0;
                pllAused = true;
                for (set<unsigned long>::iterator it6 = clk6freqs.begin(); it6 != clk6freqs.end(); ++it6)
                {
                    availableFrequenciesPLLA.insert(pair<unsigned long, int>(*it6, 0));
                }
            }
            if (!clk7satisfied)
            {
                clocks[7].pllSource = 1;
                pllBused = true;
                for (set<unsigned long>::iterator it7 = clk7freqs.begin(); it7 != clk7freqs.end(); ++it7)
                {
                    availableFrequenciesPLLB.insert(pair<unsigned long, int>(*it7, 0));
                }
            }
        }
    }

    //PLLA stage, find  all clocks that have integer coefficients with PLLA
    //if pllA is not used by clk6 or clk7, fill available frequencies according to clk1-clk5 clocks
    if (availableFrequenciesPLLA.size() == 0 && !pllAused)
    {
        for (int i = 0; i < 6; ++i)
        {
            unsigned long freq =
                clocks[i].outputFreqHz > Fmin
                    ? clocks[i].outputFreqHz
                    : (clocks[i].outputFreqHz * ((Fmin / clocks[i].outputFreqHz) + ((Fmin % clocks[i].outputFreqHz) != 0)));
            while (freq >= Fmin && freq <= Fmax)
            {
                //add all output frequency multiples that are in VCO interval
                availableFrequenciesPLLA.insert(pair<unsigned long, int>(freq, 0));
                freq += clocks[i].outputFreqHz;
            }
        }
    }

    int bestScore = 0; //score shows how many outputs have integer dividers
    //calculate scores for all available frequencies
    unsigned long bestVCOA = 0;
    for (map<unsigned long, int>::iterator it = availableFrequenciesPLLA.begin(); it != availableFrequenciesPLLA.end(); ++it)
    {
        for (int i = 0; i < 8; ++i)
        {
            if (clocks[i].outputFreqHz == 0 || !clocks[i].powered)
                continue;

            if ((it->first % clocks[i].outputFreqHz) == 0)
            {
                it->second = it->second + 1;
            }
        }
        if (it->second >= bestScore)
        {
            bestScore = it->second;
            bestVCOA = it->first;
        }
    }

    plls[0].VCO_Hz = bestVCOA;
    plls[0].feedbackDivider = (double)bestVCOA / plls[0].inputFreqHz;

    for (int i = 0; i < clockCount; ++i)
    {
        if (clocks[i].outputFreqHz == 0 || !clocks[i].powered)
            continue;

        clocks[i].multisynthDivider = bestVCOA / clocks[i].outputFreqHz;
        if (bestVCOA % clocks[i].outputFreqHz == 0)
        {
            clocks[i].int_mode = true;
            clocks[i].multisynthDivider = bestVCOA / clocks[i].outputFreqHz;
        }
        else
        {
            clocks[i].int_mode = false;
            clocks[i].multisynthDivider = (double)bestVCOA / clocks[i].outputFreqHz;
        }
        clocks[i].pllSource = 0;
    }

    //PLLB stage, find  all clocks that have integer coefficients with PLLB
    //if pllB is not used by clk6 or clk7, fill available frequencies according to clk1-clk5 clocks, that don't have integer dividers
    if (availableFrequenciesPLLB.size() == 0 && !pllBused)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (clocks[i].outputFreqHz == 0 || !clocks[i].powered)
                continue;

            if (clocks[i].int_mode) //skip clocks with integer dividers
                continue;
            unsigned long freq =
                clocks[i].outputFreqHz > Fmin
                    ? clocks[i].outputFreqHz
                    : (clocks[i].outputFreqHz * ((Fmin / clocks[i].outputFreqHz) + ((Fmin % clocks[i].outputFreqHz) != 0)));
            while (freq >= Fmin && freq <= Fmax)
            {
                availableFrequenciesPLLB.insert(pair<unsigned long, int>(freq, 0));
                freq += clocks[i].outputFreqHz;
            }
        }
    }

    bestScore = 0;
    //calculate scores for all available frequencies
    unsigned long bestVCOB = 0;
    for (map<unsigned long, int>::iterator it = availableFrequenciesPLLB.begin(); it != availableFrequenciesPLLB.end(); ++it)
    {
        for (int i = 0; i < 8; ++i)
        {
            if (clocks[i].outputFreqHz == 0 || !clocks[i].powered)
                continue;

            if ((it->first % clocks[i].outputFreqHz) == 0)
            {
                it->second = it->second + 1;
            }
        }
        if (it->second >= bestScore)
        {
            bestScore = it->second;
            bestVCOB = it->first;
        }
    }

    if (bestVCOB == 0) //just in case if pllb is not used make it the same frequency as plla
        bestVCOB = bestVCOA;
    plls[1].VCO_Hz = bestVCOB;
    plls[1].feedbackDivider = static_cast<double>(bestVCOB) / plls[0].inputFreqHz;
    for (int i = 0; i < clockCount; ++i)
    {
        if (clocks[i].outputFreqHz == 0 || !clocks[i].powered)
            continue;

        if (clocks[i].int_mode)
            continue;
        clocks[i].multisynthDivider = bestVCOB / clocks[i].outputFreqHz;
        if (bestVCOB % clocks[i].outputFreqHz == 0)
        {
            clocks[i].int_mode = true;
            clocks[i].multisynthDivider = bestVCOB / clocks[i].outputFreqHz;
        }
        else
        {
            clocks[i].int_mode = false;
            clocks[i].multisynthDivider = static_cast<double>(bestVCOB) / clocks[i].outputFreqHz;
        }
        clocks[i].pllSource = 1;
    }
}

/** @brief Modifies register map with clock settings
    @return true if success
*/
Si5351C::Status Si5351C::ConfigureClocks()
{
    FindVCO(CLK, PLL, 600000000, 900000000);
    int addr;
    m_newConfiguration[3] = std::byte{ 0 };
    for (uint8_t i = 0; i < 8; ++i)
    {
        m_newConfiguration[3] |= static_cast<std::byte>((!CLK[i].powered) << i); //enabled
        m_newConfiguration[16 + i] = std::byte{ 0 };
        m_newConfiguration[16 + i] |= static_cast<std::byte>(!CLK[i].powered << 7); // powered

        if (CLK[i].int_mode)
        {
            m_newConfiguration[16 + i] |= std::byte{ 1 << 6 }; //integer mode
        }
        else
        {
            m_newConfiguration[16 + i] |= std::byte{ 0 << 6 };
        }

        m_newConfiguration[16 + i] |= static_cast<std::byte>(CLK[i].pllSource << 5); //PLL source
        m_newConfiguration[16 + i] |= static_cast<std::byte>(CLK[i].inverted << 4); // invert
        m_newConfiguration[16 + i] |= std::byte{ 3 << 2 };
        m_newConfiguration[16 + i] |= std::byte{ 3 };

        addr = 42 + i * 8;
        int DivA, DivB, DivC;
        realToFrac(CLK[i].multisynthDivider, DivA, DivB, DivC);

        lime::info("CLK%d fOut = %g MHz  Multisynth Divider %d %d/%d  R divider = %d source = %s",
            i,
            CLK[i].outputFreqHz / 1000000.0,
            DivA,
            DivB,
            DivC,
            CLK[i].outputDivider,
            (CLK[i].pllSource == 0 ? "PLLA" : "PLLB"));

        if (CLK[i].multisynthDivider < 8 || 900 < CLK[i].multisynthDivider)
        {
            lime::error("Si5351C - Output multisynth divider is outside [8;900] interval.");
            return Status::FAILED;
        }

        if (i < 6)
        {
            if (CLK[i].outputFreqHz <= 150000000)
            {
                unsigned MSX_P1 = 128 * DivA + floor(128 * ((float)DivB / DivC)) - 512;
                unsigned MSX_P2 = 128 * DivB - DivC * floor(128 * DivB / DivC);
                unsigned MSX_P3 = DivC;

                m_newConfiguration[addr] = static_cast<std::byte>(MSX_P3 >> 8);
                m_newConfiguration[addr + 1] = static_cast<std::byte>(MSX_P3);

                m_newConfiguration[addr + 2] = std::byte{ 0 };
                m_newConfiguration[addr + 2] |= static_cast<std::byte>((MSX_P1 >> 16) & 0x03);
                m_newConfiguration[addr + 3] = static_cast<std::byte>(MSX_P1 >> 8);
                m_newConfiguration[addr + 4] = static_cast<std::byte>(MSX_P1);

                m_newConfiguration[addr + 5] = static_cast<std::byte>((MSX_P2 >> 16) & 0x0F);
                m_newConfiguration[addr + 5] |= static_cast<std::byte>((MSX_P3 >> 16) << 4);

                m_newConfiguration[addr + 6] = static_cast<std::byte>(MSX_P2);
                m_newConfiguration[addr + 7] = static_cast<std::byte>(MSX_P2 >> 8);
            }
            else if (CLK[i].outputFreqHz <= 160000000) // AVAILABLE ONLY ON 0-5 MULTISYNTHS
            {
                lime::error("Si5351C - clock configuring for more than 150 MHz not implemented");
                return Status::FAILED;
            }
        }
        else // CLK6 and CLK7 only integer mode
        {
            if (CLK[i].outputFreqHz <= 150000000)
            {
                if (i == 6)
                {
                    m_newConfiguration[90] = static_cast<std::byte>(DivA);
                    if (DivA % 2 != 0)
                    {
                        lime::error("Si5351C - CLK6 multisynth divider is not even integer");
                        return Status::FAILED;
                    }
                }
                else
                {
                    m_newConfiguration[91] = static_cast<std::byte>(DivA);
                    if (DivA % 2 != 0)
                    {
                        lime::error("Si5351C - CLK7 multisynth divider is not even integer");
                        return Status::FAILED;
                    }
                }
            }
            else if (CLK[i].outputFreqHz <= 160000000) // AVAILABLE ONLY ON 0-5 MULTISYNTHS
            {
                lime::error("Si5351C - clock configuring for more than 150 MHz not implemented");
                return Status::FAILED;
            }
        }
    }

    //configure pll
    //set input clk source
    m_newConfiguration[15] = m_newConfiguration[15] & std::byte{ 0xF3 };
    m_newConfiguration[15] |= static_cast<std::byte>((PLL[0].CLK_SRC & 1) << 2);
    m_newConfiguration[15] |= static_cast<std::byte>((PLL[1].CLK_SRC & 1) << 3);
    for (int i = 0; i < 2; ++i)
    {
        addr = 26 + i * 8;
        if (PLL[i].feedbackDivider < 15 || PLL[i].feedbackDivider > 90)
        {
            lime::error("Si5351C - VCO frequency divider out of range [15:90].");
            return Status::FAILED;
        }
        if (PLL[i].VCO_Hz < 600000000 || PLL[i].VCO_Hz > 900000000)
        {
            lime::error("Si5351C - Can't calculate valid VCO frequency.");
            return Status::FAILED;
        }

        //calculate MSNx_P1, MSNx_P2, MSNx_P3
        int MSNx_P1;
        int MSNx_P2;
        int MSNx_P3;

        int DivA;
        int DivB;
        int DivC;
        realToFrac(PLL[i].feedbackDivider, DivA, DivB, DivC);
        lime::info("Si5351C: VCO%s = %g MHz  Feedback Divider %d %d/%d",
            (i == 0 ? "A" : "B"),
            PLL[i].VCO_Hz / 1000000.0,
            DivA,
            DivB,
            DivC);

        MSNx_P1 = 128 * DivA + floor(128 * (static_cast<float>(DivB) / DivC)) - 512;
        MSNx_P2 = 128 * DivB - DivC * floor(128 * DivB / DivC);
        MSNx_P3 = DivC;

        m_newConfiguration[addr + 4] = static_cast<std::byte>(MSNx_P1);
        m_newConfiguration[addr + 3] = static_cast<std::byte>(MSNx_P1 >> 8);
        m_newConfiguration[addr + 2] = static_cast<std::byte>(MSNx_P1 >> 16);

        m_newConfiguration[addr + 7] = static_cast<std::byte>(MSNx_P2);
        m_newConfiguration[addr + 6] = static_cast<std::byte>(MSNx_P2 >> 8);
        m_newConfiguration[addr + 5] = static_cast<std::byte>((MSNx_P2 >> 16) & 0x0F);

        m_newConfiguration[addr + 5] |= static_cast<std::byte>((MSNx_P3 >> 16) << 4);
        m_newConfiguration[addr + 1] |= static_cast<std::byte>(MSNx_P3);
        m_newConfiguration[addr] |= static_cast<std::byte>(MSNx_P3 >> 8);
    }
    return Status::SUCCESS;
}

/** @brief Sets output clock parameters
    @param id clock id 0-CLK0 1-CLK1 ...
    @param fOut_Hz output frequency in Hz
    @param enabled is this output powered
    @param inverted invert clock
*/
void Si5351C::SetClock(unsigned char id, unsigned long fOut_Hz, bool enabled, bool inverted)
{
    if (id < 8)
    {
        if (fOut_Hz < 8000 || fOut_Hz > 160000000)
        {
            lime::error(
                "Si5351C - CLK%d output frequency must be between 8kHz and 160MHz. fOut_MHz = %g", (int)id, fOut_Hz / 1000000.0);
            return;
        }
        CLK[id].powered = enabled;
        CLK[id].inverted = inverted;
        CLK[id].outputFreqHz = fOut_Hz;
    }
}

/** @brief Sets PLL input frequency
    @param id PLL id 0-PLLA 1-PLLB
    @param CLKIN_Hz clock input in Hz
    @param CLK_SRC source of the clock
*/
void Si5351C::SetPLL(unsigned char id, unsigned long CLKIN_Hz, int CLK_SRC)
{
    if (id < 2)
    {
        PLL[id].inputFreqHz = CLKIN_Hz;
        PLL[id].CLK_SRC = CLK_SRC;
    }
}

/** @brief Resets configuration registers to default values
*/
void Si5351C::Reset()
{
    m_newConfiguration.fill(std::byte{ 0 });

    for (const auto& value : m_defaultConfiguration)
    {
        m_newConfiguration[value.index] = value.value;
    }
}

Si5351C::StatusBits Si5351C::GetStatusBits()
{
    StatusBits stat;
    std::vector<std::byte> dataIo;
    dataIo.push_back(std::byte{ 0 });
    dataIo.push_back(std::byte{ 1 });

    try
    {
        comms.I2CRead(addrSi5351, dataIo.data(), 2);
    } catch (std::runtime_error& e)
    {
        return stat;
    }

    stat.sys_init = static_cast<int>(dataIo[0] >> 7);
    stat.lol_b = static_cast<int>((dataIo[0] >> 6) & std::byte{ 0x1 });
    stat.lol_a = static_cast<int>((dataIo[0] >> 5) & std::byte{ 0x1 });
    stat.los = static_cast<int>((dataIo[0] >> 4) & std::byte{ 0x1 });
    stat.sys_init_stky = static_cast<int>((dataIo[1] >> 7));
    stat.lol_b_stky = static_cast<int>((dataIo[1] >> 6) & std::byte{ 0x1 });
    stat.lol_a_stky = static_cast<int>((dataIo[1] >> 5) & std::byte{ 0x1 });
    stat.los_stky = static_cast<int>((dataIo[1] >> 4) & std::byte{ 0x1 });
    return stat;
}

Si5351C::Status Si5351C::ClearStatus()
{
    std::vector<std::byte> dataWr;
    dataWr.push_back(std::byte{ 1 });
    dataWr.push_back(std::byte{ 0x1 });

    try
    {
        comms.I2CWrite(addrSi5351, dataWr.data(), dataWr.size());
        return Status::SUCCESS;
    } catch (std::runtime_error& e)
    {
        lime::error("Si5351C configuration failed %s", e.what());
        return Status::FAILED;
    }
}
