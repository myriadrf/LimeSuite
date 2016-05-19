/**
@file VersionInfo.h
@author Lime Microsystems
@brief API for querying version and build information.
*/

#ifndef LIMESUITE_VERSION_INFO_H
#define LIMESUITE_VERSION_INFO_H

#include <LimeSuiteConfig.h>
#include <string>

namespace lime
{

    /*!
     * Get the library version as a dotted string.
     * The format is major.minor.patch.build-extra.
     */
    std::string GetLibraryVersion(void);

    /*!
     * Get the date of the build in "%Y-%M-%d" format.
     */
    std::string GetBuildTimestamp(void);

}

#endif //LIMESUITE_VERSION_INFO_H
