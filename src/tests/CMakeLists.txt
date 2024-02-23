set(LIME_TEST_SUITE_NAME LimeSuite2Test)

set(LIME_TEST_SUITE_SOURCES
    parsers/CoefficientFileParserTest.cpp
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

if (ENABLE_LIMESDR_X3)
    set(LIME_TEST_SUITE_SOURCES ${LIME_TEST_SUITE_SOURCES}     
        boards/LimeSDR_X3/SlaveSelectShimTest.cpp
    )
endif()

if (ENABLE_LITE_PCIE)
    set(LIME_TEST_SUITE_SOURCES ${LIME_TEST_SUITE_SOURCES}     
        comms/PCIe/PCIE_CSR_PipeTest.cpp
    )
endif()

if (ENABLE_LIMESDR_USB)
    set(LIME_TEST_SUITE_SOURCES ${LIME_TEST_SUITE_SOURCES}     
        boards/LimeSDR/LimeSDRTest.cpp
        boards/LimeSDR/USB_CSR_Pipe_SDRTest.cpp
    )
endif()

if (ENABLE_LIMESDR_MINI)
    set(LIME_TEST_SUITE_SOURCES ${LIME_TEST_SUITE_SOURCES}     
        boards/LimeSDR_Mini/LimeSDR_MiniTest.cpp
        boards/LimeSDR_Mini/USB_CSR_Pipe_MiniTest.cpp
    )
endif()

add_executable(${LIME_TEST_SUITE_NAME} ${LIME_TEST_SUITE_SOURCES})

target_include_directories(${LIME_TEST_SUITE_NAME} PUBLIC ${LIME_SUITE_INCLUDES} tests)
target_link_libraries(
    ${LIME_TEST_SUITE_NAME}
    PUBLIC
    ${MAIN_LIBRARY_NAME}
    GTest::gtest_main
    GTest::gmock_main
)

if (CMAKE_BINARY_DIR)
	set_target_properties(${LIME_TEST_SUITE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

include(GoogleTest)
gtest_discover_tests(${LIME_TEST_SUITE_NAME})

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_CODE_COVERAGE AND (NOT MSVC))
    include(CodeCoverage)
    setup_target_for_coverage_lcov(NAME ${LIME_TEST_SUITE_NAME}_coverage EXECUTABLE ${LIME_TEST_SUITE_NAME} EXCLUDE "/usr/*" "build/*" "external/*" "tests/*")
    target_link_libraries(${LIME_TEST_SUITE_NAME} PUBLIC gcov)
endif()
