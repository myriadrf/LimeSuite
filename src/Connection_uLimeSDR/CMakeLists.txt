########################################################################
## Support for USB uLimeSDR connection
########################################################################

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Connection_uLimeSDR)

if(WIN32)
    set(USB_INCLUDE_DIRS  ${THIS_SOURCE_DIR}/FTD3XXLibrary)
    if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
        set(USB_LIBRARIES ${USB_INCLUDE_DIRS}/x64/Static/FTD3XX.lib SetupAPI)
    else()
        set(USB_LIBRARIES ${USB_INCLUDE_DIRS}/Win32/Static/FTD3XX.lib SetupAPI)
    endif()
    set(USB_FOUND TRUE)
elseif(UNIX)
    find_package(libusb-1.0)
    set(USB_FOUND ${LIBUSB_1_FOUND})
    set(USB_INCLUDE_DIRS ${LIBUSB_1_INCLUDE_DIRS})
    set(USB_LIBRARIES ${LIBUSB_1_LIBRARIES})
endif()

message(STATUS "USB_INCLUDE_DIRS: ${USB_INCLUDE_DIRS}")
message(STATUS "USB_LIBRARIES: ${USB_LIBRARIES}")

set(CONNECTION_uLimeSDR_SOURCES
    ${THIS_SOURCE_DIR}/Connection_uLimeSDREntry.cpp
    ${THIS_SOURCE_DIR}/Connection_uLimeSDR.cpp
    ${THIS_SOURCE_DIR}/Connection_uLimeSDRing.cpp
)

set(CONNECTION_uLimeSDR_LIBRARIES
    ${USB_LIBRARIES}
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_uLimeSDR "Enable uLimeSDR" ON "ENABLE_LIBRARY;USB_FOUND" OFF)
add_feature_info(Connection_uLimeSDR ENABLE_uLimeSDR "uLimeSDR Connection support")
if (NOT ENABLE_uLimeSDR)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(LimeSuite PUBLIC ${USB_INCLUDE_DIRS})
target_sources(LimeSuite PUBLIC ${CONNECTION_uLimeSDR_SOURCES})
target_link_libraries(LimeSuite ${CONNECTION_uLimeSDR_LIBRARIES})
