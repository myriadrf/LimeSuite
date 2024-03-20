########################################################################
## Support for LimeSDR_XTRX board
########################################################################

set(THIS_SOURCE_DIR boards/LimeSDR_XTRX)

set(LIMESDR_XTRX_SOURCES
    ${THIS_SOURCE_DIR}/LimeSDR_XTRXEntry.cpp
    ${THIS_SOURCE_DIR}/LimeSDR_XTRX.cpp
    ${THIS_SOURCE_DIR}/FPGA_XTRX.cpp
)


########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMESDR_XTRX "Enable LimeSDR XTRX support" ON "ENABLE_LIBRARY;ENABLE_LITE_PCIE" OFF)
add_feature_info(LIMESDR_XTRX ENABLE_LIMESDR_XTRX "LimeSDR XTRX board support")
if (NOT ENABLE_LIMESDR_XTRX)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${LIMESDR_XTRX_SOURCES})
