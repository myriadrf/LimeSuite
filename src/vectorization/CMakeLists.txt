if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wpedantic)
    add_compile_options(-O3 -march=native)
    #add_compile_options(-O3 -march=native -fopt-info-vec-all)
endif()

add_library(samplesConversion STATIC samplesConversion.cpp)
target_include_directories(samplesConversion PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

add_executable(samplesConversionPerfTest main.cpp)
target_link_libraries(samplesConversionPerfTest samplesConversion)

install(TARGETS samplesConversion EXPORT samplesConversion)
install(EXPORT samplesConversion DESTINATION lib)
