########################################################################
## Support for EVB7 over USB STREAM connection
########################################################################

if(WIN32)
    find_package(CyAPI)
    set(USB_FOUND ${CYAPI_FOUND})
    set(USB_INCLUDE_DIRS ${CYAPI_INCLUDE_DIRS})
    set(USB_LIBRARIES ${CYAPI_LIBRARIES})
elseif(UNIX)
    find_package(libusb-1.0)
    set(USB_FOUND ${LIBUSB_1_FOUND})
    set(USB_INCLUDE_DIRS ${LIBUSB_1_INCLUDE_DIRS})
    set(USB_LIBRARIES ${LIBUSB_1_LIBRARIES})
endif()

message(STATUS "USB_INCLUDE_DIRS: ${USB_INCLUDE_DIRS}")
message(STATUS "USB_LIBRARIES: ${USB_LIBRARIES}")

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_EVB7STREAM "Enable EVB + STREAM" ON "USB_FOUND" OFF)
add_feature_info(EVB7STREAM ENABLE_EVB7COM "EVB + STREAM Connection support")
if (NOT ENABLE_EVB7STREAM)
    return()
endif()

########################################################################
## Build library
########################################################################
