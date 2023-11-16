########################################################################
## Support for USB STREAM connection
########################################################################
set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/comms/USB)

include(${THIS_SOURCE_DIR}/FT601/CMakeLists.txt)
include(${THIS_SOURCE_DIR}/FX3/CMakeLists.txt)

message(STATUS "USB_INCLUDE_DIRS: ${USB_INCLUDE_DIRS}")
message(STATUS "USB_LIBRARIES: ${USB_LIBRARIES}")

set(CONNECTION_FX3_SOURCES
    ${THIS_SOURCE_DIR}/FX3/FX3.cpp
    ${THIS_SOURCE_DIR}/FX3/USBTransferContext_FX3.cpp
)

set(CONNECTION_FT601_SOURCES
    ${THIS_SOURCE_DIR}/FT601/FT601.cpp
    ${THIS_SOURCE_DIR}/FT601/USBTransferContext_FT601.cpp
)

set(COMMS_USB_SOURCES
    ${THIS_SOURCE_DIR}/LMS64C_FPGA_Over_USB.cpp
    ${THIS_SOURCE_DIR}/LMS64C_LMS7002M_Over_USB.cpp
    ${THIS_SOURCE_DIR}/TRXLooper_USB.cpp
    ${THIS_SOURCE_DIR}/USBEntry.cpp
    ${THIS_SOURCE_DIR}/USBGeneric.cpp
    ${THIS_SOURCE_DIR}/USBTransferContext.cpp
    ${CONNECTION_FX3_SOURCES}
    ${CONNECTION_FT601_SOURCES}
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
target_link_libraries(${MAIN_LIBRARY_NAME} PUBLIC ${CONNECTION_FX3_LIBRARIES})
