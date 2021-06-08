set(kissFFT_src_files
	kiss_fft.c
)

add_library(kissFFT STATIC ${kissFFT_src_files})
target_include_directories(kissFFT PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET kissFFT PROPERTY POSITION_INDEPENDENT_CODE TRUE)
