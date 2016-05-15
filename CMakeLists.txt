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

#version numbers built into shared library
#todo version number, example could be date, standard major.minor.patch
#todo abi compatibility, could be version, date, incrementing number
#maybe parse version.h for info?
set(LIME_SUITE_VERSION 0.0.0)
set(LIME_SUITE_ABI 1)

########################################################################
## compiler flags
########################################################################

#gcc c++11 enable flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

#visual studio compiler flags
if (MSVC)
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
    set(SQLITE3_SRC_DIR external/sqlite-3.12)
    add_subdirectory(${SQLITE3_SRC_DIR})
    set(SQLITE3_FOUND TRUE)
    set(SQLITE3_INCLUDE_DIRS ${SQLITE3_SRC_DIR})
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
message(STATUS "Lime Suite Version: ${LIME_SUITE_VERSION}")
message(STATUS "Lime Suite ABI: ${LIME_SUITE_ABI}")

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

