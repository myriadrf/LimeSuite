set(DPDTest_src_files
	DPDTest.cpp	
)

add_library(DPDTest STATIC ${DPDTest_src_files})
target_include_directories(DPDTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DPDTest kissFFT LMS7002M)
if(WIN32)
	target_link_libraries(DPDTest ${CYAPI_LIBRARIES})
endif()
if(UNIX)
	target_link_libraries(DPDTest usb-1.0)
endif()
