/**
@file CommonUtilities.cpp
@author Lime Microsystems (www.limemicro.com)
@brief Implementation of various utility functions
*/

#include "CommonUtilities.h"
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#ifndef __unix__
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/time.h>
#endif // LINUX

#include <cmath>

/** @brief Sleeps for given amount of milliseconds
*/
void milSleep(unsigned int miliseconds)
{
#ifndef __unix__
    Sleep(miliseconds);
#else
    usleep(miliseconds*1000);
#endif
}

/** @brief Returns number of miliseconds since system was started
    @return number of miliseconds
*/
long getMilis()
{
#ifndef __unix__
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);

    double time_in_mill = (tv.tv_sec)*1000 + (tv.tv_usec)/1000;
    return time_in_mill;
#endif // LINUX
}

/** @brief Converts given integer to hex string
    @param i integer to convert
    @param sizeBytes number of bytes to print
*/
string int2hex(int i, int sizeBytes)
{
    std::stringstream stream;
    for(int k=sizeBytes; k>0; --k)
        stream << std::setfill ('0') << std::setw(2) << uppercase << std::hex << ((i >> (k-1)*8)&0xFF);
    return stream.str();
}

/** @brief Converts given integer to hex string
    @param i integer to convert
    @param sizeBytes number of bytes to print
*/
string uint2hex(unsigned int i, int sizeBytes)
{
    std::stringstream stream;
    stream << std::setfill ('0') << std::setw(2*sizeBytes) << uppercase  << std::hex << i;
    return stream.str();
}

int hex2int(const string hex)
{
    return (int)strtol(hex.c_str(), NULL, 0);
}

/** @brief Converts given integer to hex string with prefix 0x
    @param i integer to convert    
*/
string short2hex(unsigned short i)
{
    string str;
    char ctemp[16];
    sprintf(ctemp, "0x%04X", i);
    str = ctemp;
    return str;
}
