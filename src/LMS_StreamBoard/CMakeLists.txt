set(streamBoard_src_files
	LMS_StreamBoard.cpp
)

add_library(LMS_StreamBoard STATIC ${streamBoard_src_files})
target_include_directories(LMS_StreamBoard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(LMS_StreamBoard kissFFT LMS7002M ${connectionRegistryLibs})	

#we should only link the stream connection with usb libraries
#this library does not use the usb library directly
#if(WIN32 AND ENABLE_USB_CONNECTION)
#	find_package(CyAPI REQUIRED)
#	LINK_DIRECTORIES(${CYAPI_LIBRARIES})	
#	include_directories(${CYAPI_INCLUDE_DIRS})
#	target_link_libraries(LMS_StreamBoard ${CYAPI_LIBRARIES} SetupAPI)
#endif()
#if(UNIX AND ENABLE_USB_CONNECTION)
#	target_link_libraries(LMS_StreamBoard usb-1.0 -lpthread)
#endif()

set_property(TARGET LMS_StreamBoard PROPERTY POSITION_INDEPENDENT_CODE TRUE)

