
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

