/**
@file CommonUtilities.cpp
@author Lime Microsystems (www.limemicro.com)
@brief Various utility functions
*/

#ifndef LMS_COMMON_UTILITIES_H
#define LMS_COMMON_UTILITIES_H

#include <string>
using namespace std;

/** @brief Common utility functions used by logic and GUI
*/

void milSleep(unsigned int miliseconds);
long getMilis();
string int2hex(int i, int sizeBytes = 1);
string uint2hex(unsigned int i, int sizeBytes = 1);

int twoComplementToIndex(unsigned int twosCompl, int bitCount);
unsigned int indexToTwoComplement(int index, int bitCount);
int hex2int(const string hex);
string short2hex(unsigned short i);

#endif // LMS_COMMON_UTILITIES_H
