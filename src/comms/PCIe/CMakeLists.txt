########################################################################
## Support for ConnectionLitePCIe connection
########################################################################
set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/comms/PCIe)

set(COMMS_LITE_PCIE_SOURCES
    ${THIS_SOURCE_DIR}/LitePCIe.cpp
    ${THIS_SOURCE_DIR}/TRXLooper_PCIE.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LITE_PCIE "Enable Lite PCIe" ON "ENABLE_LIBRARY" OFF)
add_feature_info(ConnectionLitePCIe ENABLE_LITE_PCIE "Lite PCIe Connection support")
if (NOT ENABLE_LITE_PCIE)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${COMMS_LITE_PCIE_SOURCES})
target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${THIS_SOURCE_DIR} ${THIS_SOURCE_DIR}/kernel/)
