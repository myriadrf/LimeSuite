########################################################################
## Create lime-suite project
########################################################################
cmake_minimum_required(VERSION 3.1.3)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE TYPE INTERNAL FORCE)
project("LimeSuite")

#include modules for finding CyAPI
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

cmake_policy(SET CMP0015 OLD)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0043 NEW)
endif()

########################################################################
# gather version information
########################################################################

# extract library version number from generated version.h
# in the future, this might be recoreded directly here
# use the date stamp and build number to make a dotted version
file(READ "${PROJECT_SOURCE_DIR}/src/version.h" version_h)
string(REGEX MATCH "static const int year = 20([0-9]+)" VERSION_MATCHES "${version_h}")
set(VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "static const int month = ([0-9]+)" VERSION_MATCHES "${version_h}")
set(VERSION_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "static const int day = ([0-9]+)" VERSION_MATCHES "${version_h}")
set(VERSION_PATCH ${CMAKE_MATCH_1})
string(REGEX MATCH "static const int buildsCounter = ([0-9]+)" VERSION_MATCHES "${version_h}")
set(BUILD_NUMBER ${CMAKE_MATCH_1})
set(LIME_SUITE_LIBVER "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${BUILD_NUMBER}")

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

set(LIME_SUITE_VERSION "${LIME_SUITE_LIBVER}-${LIME_SUITE_EXTVER}")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%M-%d")

########################################################################
## compiler flags
########################################################################

#gcc c++11 enable flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

#visual studio compiler flags
if (MSVC)
    include_directories(${PROJECT_SOURCE_DIR}/external/msvc)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set( MSVC_MULTITHREAD_COMPILE_FLAGS "/MP")
    set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${MSVC_MULTITHREAD_COMPILE_FLAGS}" )
endif(MSVC)

#enable c++11 extensions for OSX
if (APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wc++11-extensions")
   add_definitions(-D__unix__=1) #we use this for unix detection, but clang does not define it
endif(APPLE)

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

