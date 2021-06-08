add_library(HPM7 STATIC HPM7_wxgui.cpp)
target_include_directories(HPM7 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(HPM7 LMS7002M)
