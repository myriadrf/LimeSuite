set(LIME_TEST_SUITE_NAME LimeSuite2Test)

set(LIME_TEST_SUITE_SOURCES 
    boards/LimeSDR/LimeSDRTest.cpp
    boards/LimeSDR/USB_CSR_Pipe_SDRTest.cpp
    boards/LimeSDR_Mini/LimeSDR_MiniTest.cpp
    boards/LimeSDR_Mini/USB_CSR_Pipe_MiniTest.cpp
    boards/LimeSDR_X3/SlaveSelectShimTest.cpp
    comms/PCIe/PCIE_CSR_PipeTest.cpp
    protocols/LMS64CProtocol/CustomParameterReadTest.cpp
    protocols/LMS64CProtocol/CustomParameterWriteTest.cpp
    protocols/LMS64CProtocol/DeviceResetTest.cpp
    protocols/LMS64CProtocol/FirmwareToDescriptorTest.cpp
    protocols/LMS64CProtocol/FPGA_SPITest.cpp
    protocols/LMS64CProtocol/GetFirmwareInfoTest.cpp
    protocols/LMS64CProtocol/GPIODirReadTest.cpp
    protocols/LMS64CProtocol/GPIODirWriteTest.cpp
    protocols/LMS64CProtocol/GPIOReadTest.cpp
    protocols/LMS64CProtocol/GPIOWriteTest.cpp
    protocols/LMS64CProtocol/LMS7002M_SPITest.cpp
)

add_executable(${LIME_TEST_SUITE_NAME} ${LIME_TEST_SUITE_SOURCES})

target_include_directories(${LIME_TEST_SUITE_NAME} PUBLIC ${LIME_SUITE_INCLUDES} tests)
target_link_libraries(
    ${LIME_TEST_SUITE_NAME}
    PUBLIC
    ${MAIN_LIBRARY_NAME}
    GTest::gtest_main
    GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${LIME_TEST_SUITE_NAME})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CodeCoverage)
    # append_coverage_compiler_flags_to_target(${LIME_TEST_SUITE_NAME})
    setup_target_for_coverage_lcov(NAME ${LIME_TEST_SUITE_NAME}_coverage EXECUTABLE ${LIME_TEST_SUITE_NAME} EXCLUDE "/usr/*" "build/*" "external/*")
    target_link_libraries(${LIME_TEST_SUITE_NAME} PUBLIC gcov)
endif()
