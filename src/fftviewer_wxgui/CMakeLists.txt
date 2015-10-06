set(fftviewer_wx_gui_src_files
	fftviewer_wxgui.cpp
	fftviewer_frFFTviewer.cpp
)

add_library(fftviewer_wxgui STATIC ${fftviewer_wx_gui_src_files})
target_include_directories(fftviewer_wxgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(fftviewer_wxgui mathplot LMS_StreamBoard LMS7002M_NOVENA LTEpackets kissFFT)
