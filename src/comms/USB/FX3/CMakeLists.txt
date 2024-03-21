if(WIN32)
    # TODO: find better solution
    set(FX3_POTENTIAL_PATH ${PROJECT_SOURCE_DIR}/${FX3_SDK_RELATIVE_PATH})
    if(EXISTS "${FX3_POTENTIAL_PATH}")
        set(FX3_SDK_PATH ${FX3_POTENTIAL_PATH})
    endif()
    find_package(CyAPI)
    set_package_properties(CyAPI PROPERTIES
        TYPE RECOMMENDED
        PURPOSE "Adds USB communication support for Cypress FX3 chip"
    )
    set(FX3_FOUND ${CYAPI_FOUND})
    set(FX3_INCLUDE_DIRS ${CYAPI_INCLUDE_DIRS})
    set(FX3_LIBRARIES ${CYAPI_LIBRARIES} SetupAPI)
    if (FX3_FOUND)
        message(STATUS "FX3_INCLUDE_DIRS: ${FX3_INCLUDE_DIRS}")
        message(STATUS "FX3_LIBRARIES: ${FX3_LIBRARIES}")
    else()
        message(WARNING "CYAPI SDK not found. Omitting Cypress FX3 USB support")
    endif()
    target_link_libraries(${MAIN_LIBRARY_NAME} PUBLIC ${FX3_LIBRARIES})

    target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${FX3_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:include>
    )
elseif(UNIX)
    find_package(libusb-1.0)
    set_package_properties(libusb-1.0 PROPERTIES
        TYPE RECOMMENDED
        PURPOSE "Adds USB communication support for Cypress FX3 chip"
    )
    set(FX3_FOUND ${LIBUSB_1_FOUND})
    if (NOT ${FX3_FOUND})
        message(WARNING "libusb-1.0 libraries not found. Omitting Cypress FX3 USB support")
    endif()

    target_link_libraries(${MAIN_LIBRARY_NAME} PUBLIC usb-1.0)
    target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${LIBUSB_1_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:include>
    )
endif()

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_USB_FX3 "Enable FX3" ON "FX3_FOUND" OFF)
add_feature_info(USB_FX3 ENABLE_USB_FX3 "USB support for Cypress FX3")
if (NOT ENABLE_USB_FX3)
    return()
endif()

set(CONNECTION_FX3_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/FX3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/USBTransferContext_FX3.cpp
)

target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${CONNECTION_FX3_SOURCES})
