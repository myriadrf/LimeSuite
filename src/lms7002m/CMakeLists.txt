cmake_minimum_required(VERSION 2.8)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE TYPE INTERNAL FORCE)
project("lms7api")
#include modules for finding FFTW and CyAPI
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
cmake_policy(SET CMP0015 OLD)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
	cmake_policy(SET CMP0043 NEW)
endif()
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
if(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cpp-feather-ini-parser/INI.h)
    message(FATAL_ERROR "cpp-feather-ini-parser submodule not configured\n$ git submodule init && git submodule update")
endif ()

# set up include-directories
include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_BINARY_DIR}"  
  cpp-feather-ini-parser
 )

set(LMS7002M_src_files
    LMS7002M.cpp
	LMS7002M_parameters.cpp
	LMS7002M_statuses.cpp
	LMS7002M_filtersCalibration.cpp
	LMS7002M_RegistersMap.cpp
    LMS7002M_RxTxCalibrations.cpp
)

#dl libs used by dlopen in unix
list(APPEND LMS7002M_LIBRARIES ${CMAKE_DL_LIBS})

#support threaded client code
if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND LMS7002M_LIBRARIES -pthread)
endif()

add_library(LMS7002M STATIC ${LMS7002M_src_files})
target_include_directories(LMS7002M PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LMS7002M ${LMS7002M_LIBRARIES})
set_property(TARGET LMS7002M PROPERTY POSITION_INDEPENDENT_CODE TRUE)

