/**
@file ErrorReporting.h
@author Lime Microsystems
@brief API for reporting error codes and error messages.
All calls are thread-safe using thread-local storage.
*/

#ifndef LIMESUITE_ERROR_REPORTING_H
#define LIMESUITE_ERROR_REPORTING_H

#include <LimeSuiteConfig.h>
#include <cerrno>
#include <string>
#include <stdexcept>

namespace lime
{

/*!
 * Get the error code reported.
 */
int GetLastError(void);

/*!
 * Get the error code to string + any optional message reported.
 */
const char *GetLastErrorMessage(void);

/*!
 * Report a typical errno style error.
 * The resulting error message comes from strerror().
 */
void ReportError(const int code);

/*!
 * Report an error as an integer code and an associated message.
 */
void ReportError(const int code, const char *message);

/*!
 * Report an error as an integer code and an associated message.
 */
void ReportError(const int code, const std::string &message);

}

#endif //LIMESUITE_ERROR_REPORTING_H
