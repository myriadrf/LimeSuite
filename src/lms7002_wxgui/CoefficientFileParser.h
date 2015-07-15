#ifndef COEFFICIENT_FILE_PARSER_H
#define COEFFICIENT_FILE_PARSER_H

/**
@file	CoefficientFileParser.h
@author	Lime Microsystems
@brief	Just group parser functions into the same namespace.
*/

namespace Parser
{
	bool IsBlank(char);
	bool IsDigit(char);
	int getint(FILE *, int *);
	int getcoeffs(const char *, int *, int);
	int getcoeffs2(const char *, int *, int *, int);
	void saveToFile(const char *, const int *, int);
}

#endif // COEFFICIENT_FILE_PARSER_H
