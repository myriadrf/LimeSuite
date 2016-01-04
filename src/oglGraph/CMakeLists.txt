set(oglGraph_src_files
	dlgMarkers.cpp
	GLFont.cpp
	OpenGLGraph.cpp
	glew/glew.c
)
include_directories(glew)
include_directories(glew/GL)

#ignore warnings about narrowing: static font header
if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-Wno-narrowing)
endif()

ADD_DEFINITIONS(-DGLEW_STATIC)
add_library(oglGraph STATIC ${oglGraph_src_files})
target_include_directories(oglGraph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
	target_link_libraries(oglGraph GL ${wxWidgets_LIBRARIES})
endif()
