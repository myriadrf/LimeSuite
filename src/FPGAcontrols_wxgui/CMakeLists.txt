set(fpga_controls_wxgui_src_files
	FPGAcontrols_wxgui.cpp
)

add_library(FPGAcontrols_wxgui STATIC ${fpga_controls_wxgui_src_files})
target_include_directories(FPGAcontrols_wxgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(FPGAcontrols_wxgui LMS_StreamBoard LMS7002M)
