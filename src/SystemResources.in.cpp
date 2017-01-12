/**
@file SystemResources.h
@author Lime Microsystems
@brief APIs for locating system resources.
*/

#include "SystemResources.h"

#include <cstdlib> //getenv
#include <vector>
#include <sstream>

#ifdef _MSC_VER
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef __unix__
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#endif

std::string lime::getLimeSuiteRoot(void)
{
    //first check the environment variable
    const char *limeSuiteRoot = std::getenv("LIME_SUITE_ROOT");
    if (limeSuiteRoot != nullptr) return limeSuiteRoot;

    // Get the path to the current dynamic linked library.
    // The path to this library can be used to determine
    // the installation root without prior knowledge.
    #ifdef _MSC_VER
    char path[MAX_PATH];
    HMODULE hm = NULL;
    if (GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR) &lime::getLimeSuiteRoot, &hm))
    {
        const DWORD size = GetModuleFileNameA(hm, path, sizeof(path));
        if (size != 0)
        {
            const std::string libPath(path, size);
            const size_t slash0Pos = libPath.find_last_of("/\\");
            const size_t slash1Pos = libPath.substr(0, slash0Pos).find_last_of("/\\");
            if (slash0Pos != std::string::npos && slash1Pos != std::string::npos)
                return libPath.substr(0, slash1Pos);
        }
    }
    #endif

    return "@CMAKE_INSTALL_PREFIX@";
}

std::string lime::getHomeDirectory(void)
{
    //first check the HOME environment variable
    const char *userHome = std::getenv("HOME");
    if (userHome != nullptr) return userHome;

    //use unix user id lookup to get the home directory
    #ifdef __unix__
    const char *pwDir = getpwuid(getuid())->pw_dir;
    if (pwDir != nullptr) return pwDir;
    #endif

    return "";
}

/*!
 * The location for data in the user's home directory.
 */
std::string lime::getAppDataDirectory(void)
{
    //always check APPDATA (usually windows, but can be set for linux)
    const char *appDataDir = std::getenv("APPDATA");
    if (appDataDir != nullptr) return appDataDir;

    //use windows API to query for roaming app data directory
    #ifdef _MSC_VER
    char csidlAppDataDir[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, csidlAppDataDir)))
    {
        return csidlAppDataDir;
    }
    #endif

    //xdg freedesktop standard location environment variable
    #ifdef __unix__
    const char *xdgDataHome = std::getenv("XDG_DATA_HOME");
    if (xdgDataHome != nullptr) return xdgDataHome;
    #endif

    //xdg freedesktop standard location for data in home directory
    return getHomeDirectory() + ".local/share";
}

std::vector<std::string> lime::listImageSearchPaths(void)
{
    std::vector<std::string> imageSearchPaths;

    //separator for search paths in the environment variable
    #ifdef _MSC_VER
    static const char sep = ';';
    #else
    static const char sep = ':';
    #endif

    //check the environment's search path
    const char *imagePathEnv = std::getenv("LIME_IMAGE_PATH");
    if (imagePathEnv != nullptr)
    {
        std::stringstream imagePaths(imagePathEnv);
        std::string imagePath;
        while (std::getline(imagePaths, imagePath, sep))
        {
            if (imagePath.empty()) continue;
            imageSearchPaths.push_back(imagePath);
        }
    }

    //search directories in the user's home directory
    imageSearchPaths.push_back(lime::getAppDataDirectory() + "/LimeSuite/images");

    //search global installation directories

    return imageSearchPaths;
}

std::string lime::locateImageResource(const std::string &name)
{
    //@VERSION_MAJOR@.@VERSION_MINOR@
}
