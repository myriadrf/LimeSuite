
set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(lms_gfir_src_files
	${THIS_SOURCE_DIR}/corrections.c
	${THIS_SOURCE_DIR}/gfir_lms.c
	${THIS_SOURCE_DIR}/lms.c
	${THIS_SOURCE_DIR}/recipes.c
	${THIS_SOURCE_DIR}/rounding.c
)

target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${THIS_SOURCE_DIR} )
target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${lms_gfir_src_files})
