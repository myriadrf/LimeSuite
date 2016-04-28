set(lms_gfir_src_files
	corrections.c
	gfir_lms.c
	lms.c
	recipes.c
	rounding.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(lms_gfir STATIC ${lms_gfir_src_files})
set_property(TARGET lms_gfir PROPERTY POSITION_INDEPENDENT_CODE TRUE)
