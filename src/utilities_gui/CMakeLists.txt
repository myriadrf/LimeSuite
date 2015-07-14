set(connectionManager_gui_src_files	
	dlgConnectionSettings.cpp
)

set(messageLog_gui_src_files	
	pnlMiniLog.cpp
)

set(deviceinfo_gui_src_files	
	dlgDeviceInfo.cpp
)

add_library(utilities_gui STATIC utilities_gui.cpp 
	${connectionManager_gui_src_files} 
	${messageLog_gui_src_files}
	${deviceinfo_gui_src_files}
	)
target_link_libraries(utilities_gui lms7suiteEvents LMS7002M)
target_include_directories(utilities_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
