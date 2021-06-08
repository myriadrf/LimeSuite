set(protocol_src_files
    LMS64CProtocol.cpp
)

add_library(protocols STATIC ${protocol_src_files})
target_include_directories(protocols PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET protocols PROPERTY POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(protocols connectionRegistry Si5351C)
