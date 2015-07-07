set(lms7002_gui_src_files	
	lms7002_wxgui.cpp
	lms7002_mainPanel.cpp
	lms7002_pnlAFE_view.cpp
	lms7002_pnlBIAS_view.cpp
	lms7002_pnlBIST_view.cpp
	lms7002_pnlCDS_view.cpp
	lms7002_pnlCLKGEN_view.cpp
	lms7002_pnlLDO_view.cpp
	lms7002_pnlLimeLightPAD_view.cpp
	lms7002_pnlTxTSP_view.cpp
	lms7002_pnlRxTSP_view.cpp
	lms7002_pnlRBB_view.cpp
	lms7002_pnlRFE_view.cpp
	lms7002_pnlSPI_view.cpp	
	lms7002_pnlSX_view.cpp
	lms7002_pnlTBB_view.cpp
	lms7002_pnlTRF_view.cpp
	lms7002_pnlXBUF_view.cpp
	lms7002_pnlCalibrations_view.cpp
	lms7002_gui_utilities.cpp
)
add_library(lms7002_wxgui STATIC ${lms7002_gui_src_files})
target_include_directories(lms7002_wxgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lms7002_wxgui numericSlider)
target_link_libraries(lms7002_wxgui LMS7002M lms7suiteEvents)
