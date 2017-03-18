########################################################################
## Create lime-suite project
########################################################################
cmake_minimum_required(VERSION 3.1.3)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE TYPE INTERNAL FORCE)
project("LimeSuite")

option(BUILD_SHARED_LIBS "Build the shared library" ON)

#include modules for finding CyAPI
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

cmake_policy(SET CMP0015 OLD)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0043 NEW)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "")
   set(CMAKE_BUILD_TYPE "Release")
endif()

########################################################################
# gather version information
########################################################################
# The version of this library: major, minor, and patch
# Manually increment these numbers when creating releases.
# LIME_SUITE_API_VERSION in VersionInfo.h reflects the major/minor,
# and must be set manually when non-patch releases are created.
set(VERSION_MAJOR 17)
set(VERSION_MINOR 02)
set(VERSION_PATCH 2)

# the ABI compatibility number should be incremented when the ABI changes
# the format is to use the same major and minor, but to have an incrementing
# number if there are changes within the major.minor release series
set(LIME_SUITE_SOVER "${VERSION_MAJOR}.${VERSION_MINOR}-1")

# packagers may specify -DLIME_SUITE_EXTVER="foo" to replace the git hash
if (NOT LIME_SUITE_EXTVER)
    include(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
    get_git_head_revision(GITREFSPEC GITHASH)
    if (GITHASH)
        string(SUBSTRING "${GITHASH}" 0 8 GITHASH)
        set(LIME_SUITE_EXTVER "g${GITHASH}")
    else (GITHASH)
        set(LIME_SUITE_EXTVER "unknown")
    endif (GITHASH)
endif()

set(LIME_SUITE_LIBVER "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(LIME_SUITE_VERSION "${LIME_SUITE_LIBVER}-${LIME_SUITE_EXTVER}")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d")

#LIME_SUITE_ROOT is compiled into the library to locate the install base.
#By default, the LIME_SUITE_ROOT is set to the CMAKE_INSTALL_PREFIX.
#However users may overload this by specifying -DLIME_SUITE_ROOT=<path>.
set(LIME_SUITE_ROOT "${CMAKE_INSTALL_PREFIX}" CACHE PATH
    "Installation root for lime::getLimeSuiteRoot()")
file(TO_CMAKE_PATH "${LIME_SUITE_ROOT}" LIME_SUITE_ROOT)

########################################################################
## compiler flags
########################################################################

#C++11 is a required language feature for this project
set(CMAKE_CXX_STANDARD 11)

if(CMAKE_COMPILER_IS_GNUCXX)

    #enable C++11 on older versions of cmake
    if (CMAKE_VERSION VERSION_LESS "3.1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()

    #common warnings enabled
    add_compile_options(-Wall)

    #symbols are only exported from libraries/modules explicitly
    add_compile_options(-fvisibility=hidden)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

if (MSVC)
    include_directories(${PROJECT_SOURCE_DIR}/external/msvc)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set( MSVC_MULTITHREAD_COMPILE_FLAGS "/MP")
    set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${MSVC_MULTITHREAD_COMPILE_FLAGS}" )
endif(MSVC)

if (APPLE)
   add_definitions(-D__unix__=1) #we use this for unix detection, but clang does not define it
endif(APPLE)

########################################################################
# rpath setup - http://www.cmake.org/Wiki/CMake_RPATH_handling
########################################################################
# use, i.e. don't skip the full RPATH for the build tree
option(CMAKE_SKIP_BUILD_RPATH "skip rpath build" FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
option(CMAKE_BUILD_WITH_INSTALL_RPATH "build with install rpath" FALSE)

# the RPATH to be used when installing, but only if it's not a system directory
option(CMAKE_AUTOSET_INSTALL_RPATH TRUE)
if(CMAKE_AUTOSET_INSTALL_RPATH)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")
ENDIF("${isSystemDir}" STREQUAL "-1")
endif(CMAKE_AUTOSET_INSTALL_RPATH)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
option(CMAKE_INSTALL_RPATH_USE_LINK_PATH "build with automatic rpath" TRUE)

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

########################################################################
## sqlite dependency
########################################################################
if(WIN32)
    add_subdirectory(external/sqlite-3.12)
    set(SQLITE3_FOUND TRUE)
    set(SQLITE3_INCLUDE_DIRS)
    set(SQLITE3_LIBRARIES sqlite3_static)
else()
    find_package(SQLite3)
    if (NOT SQLITE3_FOUND)
        message(SEND_ERROR "Failed to find sqlite3 devel, please install libsqlite3-dev")
    endif()

    message(STATUS "SQLITE3: ${SQLITE3_FOUND}")
    message(STATUS "SQLITE3_INCLUDE_DIRS: ${SQLITE3_INCLUDE_DIRS}")
    message(STATUS "SQLITE3_LIBRARIES: ${SQLITE3_LIBRARIES}")
endif()

########################################################################
## lime suite build
########################################################################
add_subdirectory(src)
add_subdirectory(mcu_program)

########################################################################
## SoapySDR bindings for LMS7 RFIC driver
########################################################################
add_subdirectory(SoapyLMS7)

########################################################################
## udev rules for linux usb
########################################################################
if (DEFINED UDEV_RULES_PATH)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/udev-rules/64-limesuite.rules
        DESTINATION ${UDEV_RULES_PATH})
endif()

########################################################################
## freedesktop environment integration
########################################################################
add_subdirectory(Desktop)

#########################################################################
# summary
#########################################################################
include(FeatureSummary)
message(STATUS "")
message(STATUS "######################################################")
message(STATUS "## ${PROJECT_NAME} enabled features")
message(STATUS "######################################################")
feature_summary(WHAT ENABLED_FEATURES)
message(STATUS "######################################################")
message(STATUS "## ${PROJECT_NAME} disabled features")
message(STATUS "######################################################")
feature_summary(WHAT DISABLED_FEATURES)
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Build timestamp: ${BUILD_TIMESTAMP}")
message(STATUS "Lime Suite version: ${LIME_SUITE_VERSION}")
message(STATUS "ABI/so version: ${LIME_SUITE_SOVER}")
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DownloadImages.cmake)

########################################################################
# uninstall target
########################################################################
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

#only add uninstall target if this is the top project
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

