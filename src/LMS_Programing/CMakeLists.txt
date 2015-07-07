set(LMS_Programing_src_files
	LMS_Programing.cpp
	LMS_Programing_wxgui.cpp
)

add_library(LMS_Programing STATIC ${LMS_Programing_src_files})
target_include_directories(LMS_Programing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LMS_Programing LMS7002M)
