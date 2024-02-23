if(WIN32)
    set(FTDI_FOUND TRUE)
    set(FTDI_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external/FTD3XXLibrary)

    #library directory based on x64 vs x86
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FTD3XX_LIB_DIR ${FTDI_INCLUDE_DIRS}/x64)
    else()
        set(FTD3XX_LIB_DIR ${FTDI_INCLUDE_DIRS}/Win32)
    endif()

    #default is to use static libs
    option(FTD3XX_STATIC "Use static libs for FTD3XX" TRUE)

    #when using static libs specify FTD3XX_STATIC
    if (FTD3XX_STATIC)
        set(FTDI_LIBRARIES ${FTD3XX_LIB_DIR}/Static/FTD3XX.lib SetupAPI)
        add_definitions(-DFTD3XX_STATIC)

    #when using dynamic libs install the dll to bin/
    else()
        set(FTDI_LIBRARIES ${FTD3XX_LIB_DIR}/FTD3XX.lib SetupAPI)
        install(FILES ${FTD3XX_LIB_DIR}/FTD3XX.dll DESTINATION bin)
    endif()

    message(STATUS "FTDI_INCLUDE_DIRS: ${FTDI_INCLUDE_DIRS}")
    message(STATUS "FTDI_LIBRARIES: ${FTDI_LIBRARIES}")
elseif(UNIX)
    find_package(libusb-1.0)
    set_package_properties(libusb-1.0 PROPERTIES
        TYPE RECOMMENDED
        PURPOSE "Adds USB communication support for FT601 chip"
    )

    set(FTDI_FOUND ${LIBUSB_1_FOUND})
    if (NOT ${FTDI_FOUND})
        message(WARNING "libusb-1.0 libraries not found. Omitting FTDI USB support")
    endif()
    set(FTDI_INCLUDE_DIRS ${LIBUSB_1_INCLUDE_DIRS})
    set(FTDI_LIBRARIES ${LIBUSB_1_LIBRARIES})
endif()

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_USB_FTDI "Enable FT601" ON "FTDI_FOUND" OFF)
add_feature_info(USB_FTDI ENABLE_USB_FTDI "USB support for FTDI")
if (NOT ENABLE_USB_FTDI)
    return()
endif()

set(CONNECTION_FT601_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/FT601.cpp
    ${CMAKE_CURRENT_LIST_DIR}/USBTransferContext_FT601.cpp
)

target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${FTDI_INCLUDE_DIRS} ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${CONNECTION_FT601_SOURCES})
target_link_libraries(${MAIN_LIBRARY_NAME} PUBLIC ${FTDI_LIBRARIES})
