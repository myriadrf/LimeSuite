set(fftviewer_wx_gui_src_files
	fftviewer_wxgui.cpp
	fftviewer_frFFTviewer.cpp
)

set(USE_FFTW OFF CACHE BOOL "Use libFFTW3 for calculations")
if(USE_FFTW)
	find_package(FFTW3 REQUIRED)
	include_directories(${FFTW3_INCLUDE_DIRS})
	LINK_DIRECTORIES(${FFTW3_LIBRARIES})
endif()

add_library(fftviewer_wxgui STATIC ${fftviewer_wx_gui_src_files})
target_include_directories(fftviewer_wxgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(fftviewer_wxgui mathplot LTEpackets)
if(USE_FFTW)
	target_link_libraries(fftviewer_wxgui ${FFTW3_LIBRARIES})
elseif()
	target_link_libraries(fftviewer_wxgui kissFFT)	
endif()
