cmake_minimum_required(VERSION 2.8)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE TYPE INTERNAL FORCE)
project("lms7suite")
#include modules for finding CyAPI
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
cmake_policy(SET CMP0015 OLD)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
	cmake_policy(SET CMP0043 NEW)
endif()
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
if(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

find_package(wxWidgets REQUIRED richtext xml html xrc adv aui core base gl)
include(${wxWidgets_USE_FILE})
set(wxWidgets_USE_DEBUG OFF)
set(wxWidgets_USE_UNICODE ON)
set(wxWidgets_USE_UNIVERSAL OFF)
set(wxWidgets_USE_STATIC ON)

# Add sub-directories
add_subdirectory(lms7002_wxgui)
include_directories(lms7002_wxgui)
add_subdirectory(numericSlider)
include_directories(numericSlider)
add_subdirectory(utilities_gui)
include_directories(utilities_gui)
add_subdirectory(lms7suiteEvents)
include_directories(lms7suiteEvents)
add_subdirectory(kissFFT)
include_directories(kissFFT)
add_subdirectory(fftviewer_wxgui)
include_directories(fftviewer_wxgui)
add_subdirectory(wxMathPlot-0.1.2)
include_directories(wxMathPlot-0.1.2)
add_subdirectory(LTEpackets)
include_directories(LTEpackets)
add_subdirectory(LMS_StreamBoard)
include_directories(LMS_StreamBoard)
add_subdirectory(ADF4002)
include_directories(ADF4002)
add_subdirectory(Si5351C)
include_directories(Si5351C)
add_subdirectory(LMS_Programing)
include_directories(LMS_Programing)
add_subdirectory(lms7002m)
include_directories(lms7002m)
add_subdirectory(lms7002m_mcu)
include_directories(lms7002m_mcu)

set(BINARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")

set(lms7suiteApp
	lms7suiteApp.cpp
	lms7suiteAppFrame.cpp
	lms7suiteApp_gui.cpp
	dlgAbout.cpp
	)
	
if (MSVC)
	SET( MSVC_MULTITHREAD_COMPILE_FLAGS "/MP")
	SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${MSVC_MULTITHREAD_COMPILE_FLAGS}" )
endif(MSVC)
if(WIN32)
	find_package(CyAPI REQUIRED)
	LINK_DIRECTORIES(${CYAPI_LIBRARIES})	
	include_directories(${CYAPI_INCLUDE_DIRS})
endif()

add_executable(lms7suite ${lms7suiteApp} resource.rc)

set(AUTO_UPDATE_VERSION OFF CACHE BOOL "Uses python script to increment version")
if(AUTO_UPDATE_VERSION)
	add_custom_target(updateVersion ALL COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/auto_version.py WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})	
	add_dependencies(lms7suite updateVersion)
endif()

set_target_properties(lms7suite
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
target_link_libraries(lms7suite LMS_Programing ADF4002 Si5351C lms7002_wxgui utilities_gui numericSlider fftviewer_wxgui LMS7002M lms7suiteEvents LMS_StreamBoard ${CYAPI_LIBRARIES} ${wxWidgets_LIBRARIES})

if(UNIX)	
	set(CONNECTION_MANAGER_LIBS usb-1.0 -lpthread)
	target_link_libraries(lms7suite ${CONNECTION_MANAGER_LIBS})
endif()

if (MSVC)
	set_property(TARGET lms7suite APPEND PROPERTY LINK_FLAGS /SUBSYSTEM:WINDOWS)
endif(MSVC)
