/**
@file	ADCUnits.h
@author Lime Microsystems
@brief	enumerations of ADC sensor units
*/
#pragma once

#include <array>

namespace lime {

enum eADC_UNITS { RAW, VOLTAGE, CURRENT, RESISTANCE, POWER, TEMPERATURE, ADC_UNITS_COUNT };

static const std::array<std::string, ADC_UNITS_COUNT> ADC_UNITS_TEXT{ "", "V", "A", "Ohm", "W", "C" };
static const std::string UNKNOWN{ " unknown" };

static const std::string& adcUnits2string(const unsigned units)
{
    if (units < ADC_UNITS_COUNT)
    {
        return ADC_UNITS_TEXT.at(units);
    }

    return UNKNOWN;
}

} // namespace lime
