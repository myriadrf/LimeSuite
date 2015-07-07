set(connectionManager_gui_src_files	
	dlgConnectionSettings.cpp
)
#add_library(connectionManager_gui STATIC ${connectionManager_gui_src_files} utilities_gui)
#target_include_directories(connectionManager_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(connectionManager_gui connectionManager)

set(messageLog_gui_src_files	
	pnlMiniLog.cpp
)
#add_library(messageLog_gui STATIC ${messageLog_gui_src_files} utilities_gui)
#target_include_directories(messageLog_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(messageLog_gui utilities)

add_library(utilities_gui STATIC utilities_gui.cpp ${connectionManager_gui_src_files} ${messageLog_gui_src_files})
target_link_libraries(utilities_gui lms7suiteEvents LMS7002M)
target_include_directories(utilities_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
