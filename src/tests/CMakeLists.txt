set(LIME_TEST_SUITE_NAME LimeSuite2Test)

add_executable(${LIME_TEST_SUITE_NAME}
    asserters/ISerialPortAsserter.cpp
    LMS64CProtocolTest.cpp
    SlaveSelectShimTest.cpp
    USB_CSR_Pipe_SDRTest.cpp
)

target_include_directories(${LIME_TEST_SUITE_NAME} PUBLIC ${LIME_SUITE_INCLUDES} mocks)
target_link_libraries(
    ${LIME_TEST_SUITE_NAME}
    PUBLIC
    ${MAIN_LIBRARY_NAME}
    GTest::gtest_main
    GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${LIME_TEST_SUITE_NAME})
