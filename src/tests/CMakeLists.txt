add_executable(
    LimeSuite2Test
    LimeSuite2Test.cpp
)

target_include_directories(LimeSuite2Test PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(
    LimeSuite2Test
    PUBLIC
    ${MAIN_LIBRARY_NAME}
    GTest::gtest_main
    GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(LimeSuite2Test)
