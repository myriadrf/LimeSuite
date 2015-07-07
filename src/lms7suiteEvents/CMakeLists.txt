set(lms7suiteEvents_src_files	
	lms7suiteEvents.cpp
)

add_library(lms7suiteEvents STATIC ${lms7suiteEvents_src_files})
target_include_directories(lms7suiteEvents PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
