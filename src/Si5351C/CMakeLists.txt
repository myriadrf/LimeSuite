set(si5351_src_files
	Si5351C.cpp
	Si5351C_wxgui.cpp
)

add_library(Si5351C STATIC ${si5351_src_files})
target_include_directories(Si5351C PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Si5351C LMS7002M)
set_property(TARGET Si5351C PROPERTY POSITION_INDEPENDENT_CODE TRUE)
