########################################################################
## Support for USB STREAM connection
########################################################################
set(THIS_SOURCE_DIR comms/USB)

include(${THIS_SOURCE_DIR}/FT601/CMakeLists.txt)
include(${THIS_SOURCE_DIR}/FX3/CMakeLists.txt)

if(NOT(ENABLE_USB_FTDI OR ENABLE_USB_FX3))
    return()
endif()

set(COMMS_USB_SOURCES
    ${THIS_SOURCE_DIR}/LMS64C_FPGA_Over_USB.cpp
    ${THIS_SOURCE_DIR}/LMS64C_LMS7002M_Over_USB.cpp
    ${THIS_SOURCE_DIR}/LMS64C_ADF4002_Over_USB.cpp
    ${THIS_SOURCE_DIR}/TRXLooper_USB.cpp
    ${THIS_SOURCE_DIR}/USBEntry.cpp
    ${THIS_SOURCE_DIR}/USBGeneric.cpp
    ${THIS_SOURCE_DIR}/USBTransferContext.cpp
)

########################################################################
## Add to library
########################################################################
target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${COMMS_USB_SOURCES})
