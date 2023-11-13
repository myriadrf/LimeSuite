/**
@file	ADCUnits.h
@author Lime Microsystems
@brief	enumerations of ADC sensor units
*/
#pragma once

#include <array>

namespace lime {

enum eADC_UNITS { RAW, VOLTAGE, CURRENT, RESISTANCE, POWER, TEMPERATURE, ADC_UNITS_COUNT };

static const std::array<std::string, ADC_UNITS_COUNT> adc_units_text{ "", "V", "A", "Ohm", "W", "C" };

static std::string adcUnits2string(const unsigned units)
{
    if (units < ADC_UNITS_COUNT)
        return adc_units_text[units];
    else
        return " unknown";
}

} // namespace lime
