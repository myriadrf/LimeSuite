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

// ***************************************************************
//	Get integer value from the file
//	Returns:
//		0 upon success,
//		-1 if EOF or
//		-2 if syntax error
// ***************************************************************
int CoefficientFileParser::getValue(std::ifstream& file, double& value)
{
    if (file.eof())
    {
        return static_cast<int>(ErrorCodes::END_OF_FILE);
    }

    file >> value;
    if (!file.fail())
    {
        return static_cast<int>(ErrorCodes::SUCCESS);
    }

    file.clear();
    std::string token;
    file >> token;

    std::stringstream stream(token);
    char c;

    do
    {
        stream.clear();
        stream >> c;

        switch (c)
        {
        case ',': // Treat commas as whitespace
            break;
        case '/':
            stream >> c;
            switch (c)
            {
            case '/': // "//"
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                file >> token;
                stream = std::stringstream(token);
                break;
            case '*': // "/*"
            {
                uint commentLevelsDeep = 1;

                while (commentLevelsDeep != 0)
                {
                    c = stream.peek();
                    if (stream.eof())
                    {
                        if (file.eof())
                        {
                            return static_cast<int>(ErrorCodes::END_OF_FILE);
                        }

                        file >> token;
                        stream = std::stringstream(token);
                    }

                    stream >> token;

                    std::size_t startCommentPos = token.find("/*");
                    std::size_t endCommentPos = token.find("*/");

                    while (startCommentPos != std::string::npos || endCommentPos != std::string::npos)
                    {
                        if (endCommentPos < startCommentPos) // We're out of comment section.
                        {
                            commentLevelsDeep--;

                            if (commentLevelsDeep == 0)
                            {
                                token = token.substr(endCommentPos + 2);
                                stream = std::stringstream(token);
                                c = stream.peek();
                                break;
                            }

                            endCommentPos = token.find("*/", endCommentPos + 2);

                            if (startCommentPos != std::string::npos)
                            {
                                startCommentPos = token.find("/*", endCommentPos + 2);
                            }
                        }
                        else if (startCommentPos < endCommentPos)
                        {
                            commentLevelsDeep++;
                            startCommentPos = token.find("/*", startCommentPos + 2);

                            if (endCommentPos != std::string::npos)
                            {
                                endCommentPos = token.find("*/", startCommentPos + 2);
                            }
                        }
                    }
                }
                break;
            }
            default:
                return static_cast<int>(ErrorCodes::SYNTAX_ERROR);
            }
            break;
        default:
            return static_cast<int>(ErrorCodes::SYNTAX_ERROR);
        }

        if (stream.eof())
        {
            file >> token;

            if (file.eof())
            {
                return static_cast<int>(ErrorCodes::END_OF_FILE);
            }

            stream = std::stringstream(token);
        }

        stream >> value;
    } while (stream.fail());

    if (!stream.eof())
    {
        stream >> token;

        std::size_t charsToUnget = token.size();
        for (std::size_t i = 0; i < charsToUnget; ++i)
        {
            file.unget();
        }
    }

    return static_cast<int>(ErrorCodes::SUCCESS);
}

// ***************************************************************
// Get the coefficients from a file. Return values:
//	-2	syntax error within the file
//	-3	filename is empty string
//	-4	can not open the file
//	-5	too many coefficients in the file
//	>=0 	number of the coefficients read
// ***************************************************************
int CoefficientFileParser::getcoeffs(const std::string& filename, std::vector<double>& coefficients, int max)
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
    while (index < max)
    {
        double value = 0;

        ErrorCodes returnValue = static_cast<ErrorCodes>(getValue(inputFile, value));

        switch (returnValue)
        {
        case ErrorCodes::SUCCESS:
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
            break;
        }
    }

    inputFile.close();
    return static_cast<int>(ErrorCodes::TOO_MANY_COEFFS);
}

// ***************************************************************
// Saves given coefficients to fir file
// ***************************************************************
void CoefficientFileParser::saveToFile(const std::string& filename, const std::vector<double>& coefficients)
{
    std::ofstream fout;
    fout.open(filename, std::ios::out);

    std::string fname;
    std::size_t name_pos = filename.rfind('\\');

    if (name_pos == std::string::npos)
    {
        name_pos = filename.rfind('/');
    }

    fout << "/* ******************************************************************" << std::endl;
    fout << "   FILE:\t";
    if (name_pos != std::string::npos)
    {
        fname = filename.substr(name_pos + 1);
        fout << fname << std::endl;
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
