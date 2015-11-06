add_library(LMS7002M_NOVENA STATIC lms7002m_novena_wxgui.cpp StreamerNovena.cpp)
target_include_directories(LMS7002M_NOVENA PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LMS7002M_NOVENA LMS7002M)

#only used on Novena board
set(BUILD_EIM_TEST OFF CACHE BOOL "Build binary for testing EIM on Novena Board")
if(BUILD_EIM_TEST)
    add_executable(eimtest eimTest.cpp)
    target_link_libraries(eimtest LMS7002M_NOVENA LMS7002M LMS_StreamBoard)
    target_include_directories(eimtest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()


