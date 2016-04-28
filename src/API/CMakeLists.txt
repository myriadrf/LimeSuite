
message(STATUS "")
message(STATUS "######################################################")
message(STATUS "Adding LMS API")
message(STATUS "######################################################")


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(GFIR)
include_directories(GFIR)

set(FLTK_SKIP_FLUID true)
find_package(FLTK REQUIRED)
#get fltk required flags
execute_process(COMMAND fltk-config --ldflags OUTPUT_VARIABLE FLTK_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(${FLTK_INCLUDE_DIR})

add_executable(api_test main.cpp Lms_glWindow.cpp)

target_link_libraries(api_test
		      LimeSuite
		      ${FLTK_LDFLAGS}
		      ${FLTK_LIBRARIES} 
		      ${GLUT_LIBRARY}
		      )

