########################################################################
## Support for LimeSDR board
########################################################################

if(WIN32)
    find_package(CyAPI)
    set(USB_FOUND ${CYAPI_FOUND})
    set(USB_INCLUDE_DIRS ${CYAPI_INCLUDE_DIRS})
    set(USB_LIBRARIES ${CYAPI_LIBRARIES} SetupAPI)
elseif(UNIX)
    find_package(libusb-1.0)
    set(USB_FOUND ${LIBUSB_1_FOUND})
    set(USB_INCLUDE_DIRS ${LIBUSB_1_INCLUDE_DIRS})
    set(USB_LIBRARIES ${LIBUSB_1_LIBRARIES})
endif()

if (NOT USB_FOUND)
    message(STATUS "USB libraries for LimeSDR support not found. Requires libusb-1.0 or CyAPI")
else()
    message(STATUS "USB_INCLUDE_DIRS: ${USB_INCLUDE_DIRS}")
    message(STATUS "USB_LIBRARIES: ${USB_LIBRARIES}")
endif()

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boards/LimeSDR)

set(LIMESDR_SOURCES
    ${THIS_SOURCE_DIR}/LimeSDREntry.cpp
    ${THIS_SOURCE_DIR}/LimeSDR.cpp
)

set(LIMESDR_LIBRARIES
    ${USB_LIBRARIES}
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMESDR_USB "Enable LimeSDR support" ON "ENABLE_LIBRARY;USB_FOUND" OFF)
add_feature_info(BoardLimeSDR ENABLE_LIMESDR_USB "LimeSDR board support")
if (NOT ENABLE_LIMESDR_USB)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${USB_INCLUDE_DIRS} ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${LIMESDR_SOURCES})
target_link_libraries(${MAIN_LIBRARY_NAME} ${LIMESDR_LIBRARIES})
