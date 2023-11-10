add_executable(
  LimeSuite2Test
  LimeSuite2Test.cpp
)

target_link_libraries(
  LimeSuite2Test
  GTest::gtest_main
  GTest::gmock_main
  ${PROJECT_NAME}
)

include(GoogleTest)
gtest_discover_tests(LimeSuite2Test)
