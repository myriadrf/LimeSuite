add_library(myriad7 STATIC myriad7_wxgui.cpp)
target_include_directories(myriad7 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(myriad7 LMS7002M)
