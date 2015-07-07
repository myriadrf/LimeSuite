# Locate FFTW3 library
#
# This module defines the following variables:
# FFTW3_FOUND TRUE if the FFTW3 API was found
# FFTW3_HEADER_FILE The location of the API header
# FFTW3_INCLUDE_DIRS The location of header files
# FFTW3_LIBRARIES The FFTW3 library files
if(WIN32)	
	set(FFTW3_ROOT_DIR "" CACHE PATH "Path to the FFTW3 root directory")

	if(NOT EXISTS "${FFTW3_ROOT_DIR}")
		message(SEND_ERROR
		"FFTW3 library not found. FFTW3_ROOT_DIR=${FFTW3_ROOT_DIR}")
		return()
	endif()

	find_file(FFTW3_HEADER_FILE
		NAMES
		fftw3.h
		PATHS
		"${FFTW3_ROOT_DIR}"	
		)
	mark_as_advanced(FFTW3_HEADER_FILE)

	get_filename_component(FFTW3_INCLUDE_DIRS "${FFTW3_HEADER_FILE}" PATH)

	find_file(FFTW3_LIBRARY
		NAMES
		libfftw3f-3.lib
		PATHS
		"${FFTW3_ROOT_DIR}"
		)
	mark_as_advanced(FFTW3_LIBRARY)

	if(NOT FFTW3_LIBRARY)
		message(STATUS "FFTW3 .lib file not found")
		get_filename_component(MSVC_UTILITIES ${CMAKE_LINKER} PATH)	
		find_file(FFTW3_MSVC_LIB_IMPORTER
		NAMES
		lib.exe
		PATHS
		"${MSVC_UTILITIES}"	
		)	
		if(FFTW3_MSVC_LIB_IMPORTER)
			message(STATUS "Executing command to create .lib file: ${FFTW3_MSVC_LIB_IMPORTER} /def:libfftw3f-3.def")
			execute_process(COMMAND ${FFTW3_MSVC_LIB_IMPORTER} "/def:libfftw3f-3.def" WORKING_DIRECTORY ${FFTW3_ROOT_DIR})		
		endif()
		find_library(FFTW3_LIBRARY
			NAMES
			libfftw3f-3.lib
			PATHS
			"${FFTW3_ROOT_DIR}"
			)	
		if(FFTW3_LIBRARY)
			message(STATUS ".lib file has been created")
		endif()
	mark_as_advanced(FFTW3_HEADER_FILE)
	endif()

	find_file(FFTW3_LIBRARY_DLL
	NAMES
	libfftw3f-3.dll
	PATHS
	"${FFTW3_ROOT_DIR}"
	)
	mark_as_advanced(FFTW3_LIBRARY_DLL)

	if(FFTW3_LIBRARY)
		set(FFTW3_LIBRARIES "${FFTW3_LIBRARY}")
	endif()

	if(FFTW3_INCLUDE_DIRS AND FFTW3_LIBRARIES)
		set(FFTW3_FOUND TRUE)
	endif()

	if(FFTW3_FOUND)
		set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} "${FFTW3_INCLUDE_DIRS}")
		message(STATUS "FFTW3 includes: ${FFTW3_INCLUDE_DIRS}")
		message(STATUS "FFTW3 libs: ${FFTW3_LIBRARIES}")
	endif()
	if(NOT FFTW3_FOUND AND REQUIRED)
		message(FATAL_ERROR "FFTW3 not found. Check your FFTW3_ROOT_DIR value.")
	endif()

else() #not windows
	set(FFTW3_FOUND TRUE)
	set(FFTW3_LIBRARIES "-lfftw3f")
	set(FFTW3_INCLUDE_DIRS "")
endif()
