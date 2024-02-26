########################################################################
## Support for LimeSDR_X3 board
########################################################################

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boards/LimeSDR_X3)

set(LIMESDR_X3_SOURCES
    ${THIS_SOURCE_DIR}/FPGA_X3.cpp
    ${THIS_SOURCE_DIR}/LimeSDR_X3.cpp
    ${THIS_SOURCE_DIR}/LimeSDR_X3Entry.cpp
    ${THIS_SOURCE_DIR}/SlaveSelectShim.cpp
)


########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMESDR_X3 "Enable LimeSDR X3 board support" ON "ENABLE_LIBRARY;ENABLE_LITE_PCIE" OFF)
add_feature_info(LIMESDR_X3 ENABLE_LIMESDR_X3 "LimeSDR X3 board support")
if (NOT ENABLE_LIMESDR_X3)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${LIMESDR_X3_SOURCES})
