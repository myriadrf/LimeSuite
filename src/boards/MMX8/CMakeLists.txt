########################################################################
## Support for LimeMM-X8 board
########################################################################

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boards/MMX8)

set(MM_X8_SOURCES
    ${THIS_SOURCE_DIR}/LMS64C_ADF_Over_PCIe_MMX8.cpp
    ${THIS_SOURCE_DIR}/LMS64C_FPGA_Over_PCIe_MMX8.cpp
    ${THIS_SOURCE_DIR}/LMS64C_LMS7002M_Over_PCIe_MMX8.cpp
    ${THIS_SOURCE_DIR}/MM_X8.cpp
    ${THIS_SOURCE_DIR}/MM_X8Entry.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMESDR_MMX8 "Enable LimeMM-X8 board support" ON "ENABLE_LIBRARY;ENABLE_LITE_PCIE" OFF)
add_feature_info(ENABLE_LIMESDR_MMX8 ENABLE_LIMESDR_MMX8 "LimeSDR MM-X8 board support")
if (NOT ENABLE_LIMESDR_MMX8)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${MM_X8_SOURCES})
