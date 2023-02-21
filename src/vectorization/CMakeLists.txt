if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wpedantic)
    add_compile_options(-O3 -march=native)
endif()

add_library(samplesConversion STATIC samplesConversion.cpp)
target_include_directories(samplesConversion PUBLIC ${LIME_SUITE_INCLUDES})
