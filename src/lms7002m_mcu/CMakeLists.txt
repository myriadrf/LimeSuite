set(mcu_src_files
	MCU_BD.cpp
)

add_library(LMS7002M_MCU STATIC ${mcu_src_files})
target_include_directories(LMS7002M_MCU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
