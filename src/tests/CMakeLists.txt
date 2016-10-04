include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_TESTS "Enable library test programs" OFF "ENABLE_LIBRARY" OFF)
add_feature_info(LimeSuiteTests ENABLE_TESTS "Download gtest and build test application")

if (NOT ENABLE_TESTS)
    return()
endif()

find_package(Threads REQUIRED)
include(ExternalProject)
# Download and install GoogleTest
ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/release-1.8.0.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}
    # Disable install step
    INSTALL_COMMAND ""
)

add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set gtest properties
ExternalProject_Get_Property(gtest source_dir binary_dir)
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)
include_directories("${source_dir}/googletest/include")

add_executable(tests 
    main.cpp
    streaming.cpp
)

target_link_libraries(tests
    libgtest
    LimeSuite
)

add_dependencies(tests LimeSuite)
