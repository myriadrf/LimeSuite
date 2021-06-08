set(sqlite3_src_files
	shell.c
	sqlite3.c
)

add_library(sqlite3_static STATIC ${sqlite3_src_files})
set_property(TARGET sqlite3_static PROPERTY POSITION_INDEPENDENT_CODE TRUE)
target_include_directories(sqlite3_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


