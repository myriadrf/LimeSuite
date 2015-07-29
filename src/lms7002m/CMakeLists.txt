cmake_minimum_required(VERSION 2.8)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE TYPE INTERNAL FORCE)
project("lms7api")
#include modules for finding FFTW and CyAPI
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
cmake_policy(SET CMP0015 OLD)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
	cmake_policy(SET CMP0043 NEW)
endif()
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
if(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

# set up include-directories
include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_BINARY_DIR}"  
  cpp-feather-ini-parser
 )

set(LMS7002M_src_files
    LMS7002M.cpp
	LMS7002M_parameters.cpp
	LMS7002M_statuses.cpp
	LMS7002M_filtersCalibration.cpp    
	lmsComms.cpp
)

set(ENABLE_USB_CONNECTION "YES" CACHE BOOL INTERNAL)
set(ENABLE_SPI_CONNECTION "NO" CACHE BOOL INTERNAL)
set(CONNECTION_MANAGER_DIR connectionManager)
set(connectionManager_src_files
	${CONNECTION_MANAGER_DIR}/ConnectionManager.cpp
	${CONNECTION_MANAGER_DIR}/ConnectionCOM.cpp
	${CONNECTION_MANAGER_DIR}/ConnectionManager.h
	${CONNECTION_MANAGER_DIR}/ConnectionCOM.h
)
if(ENABLE_USB_CONNECTION)
	list(APPEND connectionManager_src_files ${CONNECTION_MANAGER_DIR}/ConnectionUSB.cpp ${CONNECTION_MANAGER_DIR}/ConnectionUSB.h)
    add_definitions(-DENABLE_USB_CONNECTION)
endif()
if(ENABLE_SPI_CONNECTION)
	list(APPEND connectionManager_src_files ${CONNECTION_MANAGER_DIR}/ConnectionSPI.cpp ${CONNECTION_MANAGER_DIR}/ConnectionSPI.h)
    add_definitions(-DENABLE_SPI_CONNECTION)
endif()

add_library(LMS7002M STATIC ${LMS7002M_src_files} ${connectionManager_src_files})
target_include_directories(LMS7002M PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32 AND ENABLE_USB_CONNECTION)
	find_package(CyAPI REQUIRED)
	LINK_DIRECTORIES(${CYAPI_LIBRARIES})	
	include_directories(${CYAPI_INCLUDE_DIRS})
	set(CONNECTION_MANAGER_LIBS ${CYAPI_LIBRARIES} SetupAPI)
	target_link_libraries(LMS7002M ${CONNECTION_MANAGER_LIBS})
endif()
if(UNIX AND ENABLE_USB_CONNECTION)	
	set(CONNECTION_MANAGER_LIBS usb-1.0 -lpthread)
	target_link_libraries(LMS7002M ${CONNECTION_MANAGER_LIBS})
endif()

