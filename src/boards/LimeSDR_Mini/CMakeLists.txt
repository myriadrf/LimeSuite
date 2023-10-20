########################################################################
## Support for LimeSDR Mini v1 board
########################################################################

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

if (NOT USB_FOUND)
    message(STATUS "USB libraries for LimeSDR support not found. Requires libusb-1.0 or CyAPI")
else()
    message(STATUS "USB_INCLUDE_DIRS: ${USB_INCLUDE_DIRS}")
    message(STATUS "USB_LIBRARIES: ${USB_LIBRARIES}")
endif()

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boards/LimeSDR_Mini)

set(LIMESDR_MINI_SOURCES
    ${THIS_SOURCE_DIR}/LimeSDR_MiniEntry.cpp
    ${THIS_SOURCE_DIR}/LimeSDR_Mini.cpp
    ${THIS_SOURCE_DIR}/FPGA_Mini.cpp
)

set(LIMESDR_MINI_LIBRARIES
    ${USB_LIBRARIES}
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIMESDR_MINI "Enable LimeSDR Mini support" ON "ENABLE_LIBRARY;USB_FOUND" OFF)
add_feature_info(BoardLimeSDR_Mini ENABLE_LIMESDR_MINI "LimeSDR Mini board support")
if (NOT ENABLE_LIMESDR_MINI)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${USB_INCLUDE_DIRS} ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${LIMESDR_MINI_SOURCES})
target_link_libraries(${MAIN_LIBRARY_NAME} PUBLIC ${LIMESDR_MINI_LIBRARIES})
