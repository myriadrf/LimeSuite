/**
@file VersionInfo.cpp
@author Lime Microsystems
@brief API for querying version and build information.
*/

#include "VersionInfo.h"

std::string lime::GetLibraryVersion(void)
{
    return "@LIME_SUITE_VERSION@";
}

std::string lime::GetBuildTimestamp(void)
{
    return "@BUILD_TIMESTAMP@";
}
