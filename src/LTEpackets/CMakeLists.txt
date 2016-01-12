set(LTEpackets_src_files
	StreamerLTE.cpp	
)

add_library(LTEpackets STATIC ${LTEpackets_src_files})
target_include_directories(LTEpackets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LTEpackets kissFFT LMS7002M)

#we should only link the stream connection with usb libraries
#this library does not use the usb library directly
#if(WIN32)
#	target_link_libraries(LTEpackets ${CYAPI_LIBRARIES})
#endif()
#if(UNIX)
#	target_link_libraries(LTEpackets usb-1.0)
#endif()

set_property(TARGET LTEpackets PROPERTY POSITION_INDEPENDENT_CODE TRUE)
