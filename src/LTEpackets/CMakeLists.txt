set(LTEpackets_src_files
	StreamerLTE.cpp	
)

add_library(LTEpackets STATIC ${LTEpackets_src_files})
target_include_directories(LTEpackets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LTEpackets kissFFT LMS7002M)
if(WIN32)
	target_link_libraries(LTEpackets ${CYAPI_LIBRARIES})
endif()
if(UNIX)
	target_link_libraries(LTEpackets usb-1.0)
endif()
