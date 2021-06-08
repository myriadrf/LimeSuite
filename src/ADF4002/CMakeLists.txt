set(adf4002_src_files
	ADF4002.cpp
	ADF4002_wxgui.cpp
)

add_library(ADF4002 STATIC ${adf4002_src_files})
target_include_directories(ADF4002 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ADF4002 LMS7002M)
