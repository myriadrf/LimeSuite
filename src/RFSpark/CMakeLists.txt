add_library(RFSpark STATIC RFSpark_wxgui.cpp)
target_include_directories(RFSpark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(RFSpark LMS7002M)
