########################################################################
## Support for LimeSDR Mini v1 board
########################################################################
set(THIS_SOURCE_DIR boards/LimeSDR_Mini)

set(LIMESDR_MINI_SOURCES
    ${THIS_SOURCE_DIR}/LimeSDR_MiniEntry.cpp
    ${THIS_SOURCE_DIR}/LimeSDR_Mini.cpp
    ${THIS_SOURCE_DIR}/FPGA_Mini.cpp
    ${THIS_SOURCE_DIR}/USB_CSR_Pipe_Mini.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMESDR_MINI "Enable LimeSDR Mini support" ON "ENABLE_LIBRARY;ENABLE_USB_FTDI" OFF)
add_feature_info(LIMESDR_MINI ENABLE_LIMESDR_MINI "LimeSDR Mini board support")
if (NOT ENABLE_LIMESDR_MINI)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${USB_INCLUDE_DIRS} ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${LIMESDR_MINI_SOURCES})
