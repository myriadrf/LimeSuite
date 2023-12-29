#ifndef COEFFICIENT_FILE_PARSER_H
#define COEFFICIENT_FILE_PARSER_H

/**
@file	CoefficientFileParser.h
@author	Lime Microsystems
@brief	The FIR coefficient file parser.
*/

#include <iostream>
#include <string>
#include <vector>

class CoefficientFileParser
{
  public:
    static int getcoeffs(const std::string& filename, std::vector<double>& coefficients, int max);
    static void saveToFile(const std::string& filename, const std::vector<double>& coefficients);

    enum class ErrorCodes : int8_t {
        SUCCESS = 0,
        END_OF_FILE = -1,
        SYNTAX_ERROR = -2,
        EMPTY_FILENAME = -3,
        UNOPENABLE_FILE = -4,
        TOO_MANY_COEFFS = -5,
    };

  private:
    static int getValue(std::ifstream& file, double& value);
};

#endif // COEFFICIENT_FILE_PARSER_H
