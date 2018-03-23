if(DEFINED INCLUDED_LIMESUITE_CONFIG_CMAKE)
  return()
endif()
set(INCLUDED_LIMESUITE_CONFIG_CMAKE TRUE)

########################################################################
# LimeSuiteConfig - cmake project configuration for client clibraries
#
# The following will be set after find_package(LimeSuite):
# LimeSuite_LIBRARIES    - development libraries
# LimeSuite_INCLUDE_DIRS - development includes
########################################################################

########################################################################
## installation root
########################################################################
get_filename_component(LIMESUITE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

########################################################################
## locate the library
########################################################################
find_library(
  LIMESUITE_LIBRARY LimeSuite
  PATHS ${LIMESUITE_ROOT}/lib${LIB_SUFFIX}
  PATH_SUFFIXES ${CMAKE_LIBRARY_ARCHITECTURE}
  NO_DEFAULT_PATH
  )
if(NOT LIMESUITE_LIBRARY)
  message(FATAL_ERROR "cannot find LimeSuite library in ${LIMESUITE_ROOT}/lib${LIB_SUFFIX}")
endif()
set(LimeSuite_LIBRARIES ${LIMESUITE_LIBRARY})

########################################################################
## locate the includes
########################################################################
find_path(
  LIMESUITE_INCLUDE_DIR lime/lms7_device.h
  PATHS ${LIMESUITE_ROOT}/include
  NO_DEFAULT_PATH
)
if(NOT LIMESUITE_INCLUDE_DIR)
  message(FATAL_ERROR "cannot find LimeSuite includes in ${LIMESUITE_ROOT}/include")
endif()
set(LimeSuite_INCLUDE_DIRS ${LIMESUITE_INCLUDE_DIR})

