########################################################################
## Support for LimeSDR_5GRadio board
########################################################################

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boards/LimeSDR_5GRadio)

set(LIMESDR_5G_SOURCES
    ${THIS_SOURCE_DIR}/LimeSDR_5GRadioEntry.cpp
    ${THIS_SOURCE_DIR}/LimeSDR_5GRadio.cpp
    ${THIS_SOURCE_DIR}/FPGA_5G.cpp
)


########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMESDR_5G "Enable LimeSDR 5GRadio support" ON "ENABLE_LIBRARY" OFF)
add_feature_info(BoardLimeSDR5G ENABLE_LIMESDR_5G "LimeSDR board support")
if (NOT ENABLE_LIMESDR_5G)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${LIMESDR_5G_SOURCES})
