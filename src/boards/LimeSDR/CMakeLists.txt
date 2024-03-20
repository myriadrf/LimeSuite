########################################################################
## Support for LimeSDR board
########################################################################
set(THIS_SOURCE_DIR boards/LimeSDR)

set(LIMESDR_SOURCES
    ${THIS_SOURCE_DIR}/LimeSDREntry.cpp
    ${THIS_SOURCE_DIR}/LimeSDR.cpp
    ${THIS_SOURCE_DIR}/USB_CSR_Pipe_SDR.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMESDR_USB "Enable LimeSDR support" ON "ENABLE_LIBRARY;ENABLE_USB_FX3" OFF)
add_feature_info(LIMESDR_USB ENABLE_LIMESDR_USB "LimeSDR USB board support")
if (NOT ENABLE_LIMESDR_USB)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${USB_INCLUDE_DIRS} ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${LIMESDR_SOURCES})
target_link_libraries(${MAIN_LIBRARY_NAME} PRIVATE ${LIMESDR_LIBRARIES})
