########################################################################
## SoapySDR bindings for IConnection
########################################################################

find_package(SoapySDR CONFIG)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_SOAPYICONNECTION "Enable SoapySDR bindings" ON "SoapySDR_FOUND" OFF)
add_feature_info(SoapyIConnection ENABLE_SOAPYICONNECTION "SoapySDR bindings for IConnection")
if (NOT ENABLE_SOAPYICONNECTION)
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
    TARGET LimeIConnection
    SOURCES
        Registration.cpp
        Settings.cpp
        Streaming.cpp
    LIBRARIES
        LMS7002M
        ${connectionRegistryLibs}
        LMS_StreamBoard
)
