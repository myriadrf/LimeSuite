if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wpedantic)
    add_compile_options(-O3 -march=native)
    #add_compile_options(-O3 -march=native -fopt-info-vec-all)
endif()

target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${THIS_SOURCE_DIR})
target_sources(${MAIN_LIBRARY_NAME} PRIVATE samplesConversion.cpp)

add_executable(samplesConversionPerfTest main.cpp)
target_link_libraries(samplesConversionPerfTest ${MAIN_LIBRARY_NAME})
