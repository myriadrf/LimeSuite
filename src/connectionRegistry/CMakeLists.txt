set(connection_registry_src_files
    ConnectionRegistry.cpp
    IConnection.cpp
)

add_library(connection_registry STATIC ${connection_registry_src_files})
target_include_directories(connection_registry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
