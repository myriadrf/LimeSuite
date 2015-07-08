set(streamBoard_src_files
	LMS_StreamBoard.cpp
)

add_library(LMS_StreamBoard STATIC ${streamBoard_src_files})
target_include_directories(LMS_StreamBoard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(LMS_StreamBoard kissFFT LMS7002M ${CONNECTION_MANAGER_LIBS})	

if(WIN32 AND ENABLE_USB_CONNECTION)
	find_package(CyAPI REQUIRED)
	LINK_DIRECTORIES(${CYAPI_LIBRARIES})	
	include_directories(${CYAPI_INCLUDE_DIRS})
	set(CONNECTION_MANAGER_LIBS ${CYAPI_LIBRARIES} SetupAPI)
	target_link_libraries(LMS_StreamBoard ${CONNECTION_MANAGER_LIBS})	
endif()
if(UNIX AND ENABLE_USB_CONNECTION)	
	set(CONNECTION_MANAGER_LIBS usb-1.0 -lpthread)
	target_link_libraries(LMS_StreamBoard ${CONNECTION_MANAGER_LIBS})
endif()


