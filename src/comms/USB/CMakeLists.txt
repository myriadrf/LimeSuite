########################################################################
## Support for USB STREAM connection
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

message(STATUS "USB_INCLUDE_DIRS: ${USB_INCLUDE_DIRS}")
message(STATUS "USB_LIBRARIES: ${USB_LIBRARIES}")

set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/comms/USB)

set(CONNECTION_FX3_SOURCES
    ${THIS_SOURCE_DIR}/FX3.cpp
)

set(COMMS_USB_SOURCES
    ${THIS_SOURCE_DIR}/TRXLooper_USB.cpp
    ${CONNECTION_FX3_SOURCES}
)

set(CONNECTION_FX3_LIBRARIES
    ${USB_LIBRARIES}
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_FX3 "Enable FX3" ON "ENABLE_LIBRARY;USB_FOUND" OFF)
add_feature_info(ConnectionFX3 ENABLE_FX3 "FX3 Connection support")
if (NOT ENABLE_FX3)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${USB_INCLUDE_DIRS} ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${COMMS_USB_SOURCES})
# target_link_libraries(${MAIN_LIBRARY_NAME} ${CONNECTION_FX3_LIBRARIES})
