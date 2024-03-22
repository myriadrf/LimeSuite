########################################################################
## Support for ConnectionLitePCIe connection
########################################################################
set(THIS_SOURCE_DIR comms/PCIe)

set(COMMS_LITE_PCIE_SOURCES
    ${THIS_SOURCE_DIR}/AvgRmsCounter.cpp
    ${THIS_SOURCE_DIR}/LMS64C_FPGA_Over_PCIe.cpp
    ${THIS_SOURCE_DIR}/LMS64C_LMS7002M_Over_PCIe.cpp
    ${THIS_SOURCE_DIR}/LitePCIe.cpp
    ${THIS_SOURCE_DIR}/PCIE_CSR_Pipe.cpp
    ${THIS_SOURCE_DIR}/TRXLooper_PCIE.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LITE_PCIE "Enable Lite PCIe" ON "UNIX;ENABLE_LIBRARY" OFF)
add_feature_info(LITE_PCIE ENABLE_LITE_PCIE "PCIe support")
if (NOT ENABLE_LITE_PCIE)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${COMMS_LITE_PCIE_SOURCES})
target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${THIS_SOURCE_DIR} ${THIS_SOURCE_DIR}/software/user/liblitepcie ${THIS_SOURCE_DIR}/software/kernel)
