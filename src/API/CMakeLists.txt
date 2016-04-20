
message(STATUS "")
message(STATUS "######################################################")
message(STATUS "Adding LMS API")
message(STATUS "######################################################")


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(GFIR)
include_directories(GFIR)
set(LMS_API_SRC
    lms7_api.cpp
    lms7_device.cpp
)
add_library(lmsAPI SHARED ${LMS_API_SRC})

target_link_libraries(lmsAPI LimeSuite lms_gfir)  

set(FLTK_SKIP_FLUID true)
find_package(FLTK REQUIRED)
#get fltk required flags
execute_process(COMMAND fltk-config --ldflags OUTPUT_VARIABLE FLTK_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(${FLTK_INCLUDE_DIR})

add_executable(api_test main.cpp Lms_glWindow.cpp)

target_link_libraries(api_test
		      lmsAPI
		      ${FLTK_LDFLAGS}
		      ${FLTK_LIBRARIES} 
		      ${GLUT_LIBRARY}
		      )

