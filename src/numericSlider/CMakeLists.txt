set(numericSlider_src_files
	numericSlider.cpp
)
add_library(numericSlider STATIC ${numericSlider_src_files})
target_include_directories(numericSlider PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})