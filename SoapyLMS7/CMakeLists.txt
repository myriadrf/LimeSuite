########################################################################
## SoapySDR bindings for LMS7
########################################################################

find_package(SoapySDR CONFIG)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_SOAPY_LMS7 "Enable LMS7 SoapySDR bindings" ON "ENABLE_LIBRARY;SoapySDR_FOUND" OFF)
add_feature_info(SoapySDRLMS7 ENABLE_SOAPY_LMS7 "SoapySDR bindings for LMS7")
if (NOT ENABLE_SOAPY_LMS7)
    return()
endif()

########################################################################
## Build support module
########################################################################
if(CMAKE_COMPILER_IS_GNUCXX)

    #disable warnings for unused parameters
    add_definitions(-Wno-unused-parameter)

endif(CMAKE_COMPILER_IS_GNUCXX)

SOAPY_SDR_MODULE_UTIL(
    TARGET LMS7Support
    SOURCES
        Registration.cpp
        Settings.cpp
        Streaming.cpp
    LIBRARIES
        LimeSuite
)
