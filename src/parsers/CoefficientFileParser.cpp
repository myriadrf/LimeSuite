/**
@file	CoefficientFileParser.cpp
@author	Lime Microsystems
@brief	Coefficient file parser functions
*/

#include "CoefficientFileParser.h"

#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace lime;

void CoefficientFileParser::parseMultilineComments(std::ifstream& file, std::string& token)
{
    token = token.substr(2);
    if (token.size() == 0)
    {
        if (file.eof())
        {
            token = "";
            return;
        }

        file >> token;
    }

    uint commentLevelsDeep = 1;

    while (commentLevelsDeep != 0) // Multiline comments can be nested
    {
        std::size_t startCommentPos = token.find("/*");
        std::size_t endCommentPos = token.find("*/");

        // While we're still finding comment starts and ends in this token.
        while ((startCommentPos != std::string::npos || endCommentPos != std::string::npos) && commentLevelsDeep > 0)
        {
            if (endCommentPos < startCommentPos) // We're out of comment section.
            {
                commentLevelsDeep--;

                token = token.substr(endCommentPos + 2);

                endCommentPos = token.find("*/");
                if (startCommentPos != std::string::npos)
                {
                    startCommentPos = token.find("/*");
                }
            }
            else if (startCommentPos < endCommentPos)
            {
                commentLevelsDeep++;

                token = token.substr(startCommentPos + 2);

                startCommentPos = token.find("/*");
                if (endCommentPos != std::string::npos)
                {
                    endCommentPos = token.find("*/");
                }
            }
        }

        if (commentLevelsDeep != 0) // Keep reading until the end of the comment(s)
        {
            if (file.eof())
            {
                token = "";
                return;
            }

            file >> token;
        }
    }
}

// ***************************************************************
//	Get integer value from the file
//	Returns:
//		0 upon success,
//		-1 if EOF or
//		-2 if syntax error
// ***************************************************************
CoefficientFileParser::ErrorCodes CoefficientFileParser::getValue(std::ifstream& file, double& value)
{
    file >> value;
    if (!file.fail())
    {
        return ErrorCodes::SUCCESS;
    }

    if (file.eof())
    {
        return ErrorCodes::END_OF_FILE;
    }

    file.clear();
    std::string token;
    file >> token;

    bool hasValueBeenRead = false;

    while (!hasValueBeenRead)
    {
        if (token.find(',') == 0) // Treat comma as a whitespace character.
        {
            token = token.substr(1);
        }
        else if (token.find("//") == 0) // Standard single line comment
        {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (file.eof())
            {
                return ErrorCodes::END_OF_FILE;
            }

            file >> token;
        }
        else if (token.find("/*") == 0) // Multiline comments
        {
            parseMultilineComments(file, token);
        }
        else
        {
            return ErrorCodes::SYNTAX_ERROR;
        }

        if (token.size() == 0)
        {
            if (file.eof())
            {
                return ErrorCodes::END_OF_FILE;
            }

            file >> token;
        }

        std::stringstream stream(token);
        stream >> value;

        if (!stream.fail())
        {
            hasValueBeenRead = true;
            stream >> token;

            if (stream.fail())
            {
                token = "";
            }
        }
    }

    // Rewind file by the amount of unparsed characters.
    std::size_t charsToUnget = token.size();
    for (std::size_t i = 0; i < charsToUnget; ++i)
    {
        file.unget();
    }

    return ErrorCodes::SUCCESS;
}

// ***************************************************************
// Get the coefficients from a file. Return values:
//	-2	syntax error within the file
//	-3	filename is empty string
//	-4	can not open the file
//	-5	too many coefficients in the file
//	>=0 	number of the coefficients read
// ***************************************************************
int CoefficientFileParser::getCoefficients(const std::string& filename, std::vector<double>& coefficients, int max)
{
    if (filename == "")
    {
        return static_cast<int>(ErrorCodes::EMPTY_FILENAME);
    }

    std::ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        return static_cast<int>(ErrorCodes::UNOPENABLE_FILE);
    }

    int index = 0;
    while (index <= max)
    {
        double value = 0;

        ErrorCodes returnValue = getValue(inputFile, value);

        switch (returnValue)
        {
        case ErrorCodes::SUCCESS:
            if (index >= max)
            {
                inputFile.close();
                return static_cast<int>(ErrorCodes::TOO_MANY_COEFFS);
            }

            coefficients[index] = value;
            index++;
            break;
        case ErrorCodes::END_OF_FILE:
            inputFile.close();
            return index;
        case ErrorCodes::SYNTAX_ERROR:
            inputFile.close();
            return static_cast<int>(ErrorCodes::SYNTAX_ERROR);
        default:
            throw std::logic_error("Unexpected return from getValue");
        }
    }

    throw std::logic_error("Unreachable; clearing a warning");
}

// ***************************************************************
// Saves given coefficients to fir file
// ***************************************************************
void CoefficientFileParser::saveToFile(const std::string& filename, const std::vector<double>& coefficients)
{
    std::ofstream fout;
    fout.open(filename, std::ios::out);

    std::size_t namePosition = filename.rfind('\\');

    if (namePosition == std::string::npos)
    {
        namePosition = filename.rfind('/');
    }

    fout << "/* ******************************************************************" << std::endl;
    fout << "   FILE:\t";
    if (namePosition != std::string::npos)
    {
        fout << filename.substr(namePosition + 1) << std::endl;
    }
    else
    {
        fout << filename << std::endl;
    }

    fout << "   DESCRIPTION:\t" << std::endl;
    fout << "   DATE:\t" << std::endl;
    fout << "   REVISIONS:\t" << std::endl;
    fout << "   ****************************************************************** */" << std::endl << std::endl;

    const std::size_t coefficientCount = coefficients.size();
    for (std::size_t i = 0; i < coefficientCount; ++i)
    {
        fout << "\t" << std::fixed << coefficients[i];

        if (i < coefficientCount - 1) // If not last
        {
            fout << ',' << std::endl;
        }
    }

    fout.close();
}