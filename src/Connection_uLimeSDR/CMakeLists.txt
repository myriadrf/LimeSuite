########################################################################
## Support for USB uLimeSDR connection
########################################################################

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Connection_uLimeSDR)

if(WIN32)
    set(USB_FOUND TRUE)
    set(USB_INCLUDE_DIRS  ${THIS_SOURCE_DIR}/FTD3XXLibrary)

    #library directory based on x64 vs x86
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FTD3XX_LIB_DIR ${USB_INCLUDE_DIRS}/x64)
    else()
        set(FTD3XX_LIB_DIR ${USB_INCLUDE_DIRS}/Win32)
    endif()

    #default is to use static libs
    option(FTD3XX_STATIC "Use static libs for FTD3XX" TRUE)

    #when using static libs specify FTD3XX_STATIC
    if (FTD3XX_STATIC)
        set(USB_LIBRARIES ${FTD3XX_LIB_DIR}/Static/FTD3XX.lib SetupAPI)
        add_definitions(-DFTD3XX_STATIC)

    #when using dynamic libs install the dll to bin/
    else()
        set(USB_LIBRARIES ${FTD3XX_LIB_DIR}/FTD3XX.lib SetupAPI)
        install(FILES ${FTD3XX_LIB_DIR}/FTD3XX.dll DESTINATION bin)
    endif()

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
target_sources(LimeSuite PRIVATE ${CONNECTION_uLimeSDR_SOURCES})
target_link_libraries(LimeSuite ${CONNECTION_uLimeSDR_LIBRARIES})
