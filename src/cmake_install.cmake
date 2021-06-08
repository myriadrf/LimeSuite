# Install script for directory: /home/srdjan/Work2/BBC/LimeSuite/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lime" TYPE FILE FILES
    "/home/srdjan/Work2/BBC/LimeSuite/src/lime/LimeSuite.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/VersionInfo.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/Logger.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/SystemResources.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/LimeSuiteConfig.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/ADF4002/ADF4002.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/lms7002m_mcu/MCU_BD.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/lms7002m_mcu/MCU_File.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/ConnectionRegistry/IConnection.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/ConnectionRegistry/ConnectionHandle.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/ConnectionRegistry/ConnectionRegistry.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/lms7002m/LMS7002M.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/lms7002m/LMS7002M_RegistersMap.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/lime/LMS7002M_parameters.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/protocols/Streamer.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/protocols/ADCUnits.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/protocols/LMS64CCommands.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/protocols/LMS64CProtocol.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/protocols/LMSBoards.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/protocols/dataTypes.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/protocols/fifo.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/Si5351C/Si5351C.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/FPGA_common/FPGA_common.h"
    "/home/srdjan/Work2/BBC/LimeSuite/src/API/lms7_device.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLimeSuite.so.18.10.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLimeSuite.so.18.10-1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLimeSuite.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/srdjan/Work2/BBC/LimeSuite/src/libLimeSuite.so.18.10.0"
    "/home/srdjan/Work2/BBC/LimeSuite/src/libLimeSuite.so.18.10-1"
    "/home/srdjan/Work2/BBC/LimeSuite/src/libLimeSuite.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLimeSuite.so.18.10.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLimeSuite.so.18.10-1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libLimeSuite.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/LimeSuiteGUI" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/LimeSuiteGUI")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/LimeSuiteGUI"
         RPATH "/usr/local/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/srdjan/Work2/BBC/LimeSuite/bin/LimeSuiteGUI")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/LimeSuiteGUI" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/LimeSuiteGUI")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/LimeSuiteGUI"
         OLD_RPATH "/home/srdjan/Work2/BBC/LimeSuite/src:/usr/local/lib:"
         NEW_RPATH "/usr/local/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/LimeSuiteGUI")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/srdjan/Work2/BBC/LimeSuite/src/LimeSuite.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/srdjan/Work2/BBC/LimeSuite/src/GFIR/cmake_install.cmake")
  include("/home/srdjan/Work2/BBC/LimeSuite/src/oglGraph/cmake_install.cmake")
  include("/home/srdjan/Work2/BBC/LimeSuite/src/utilities/cmake_install.cmake")
  include("/home/srdjan/Work2/BBC/LimeSuite/src/examples/cmake_install.cmake")

endif()

