# Install script for directory: /home/srdjan/Work/LimeSuite/dpd_test2/src

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
    set(CMAKE_INSTALL_CONFIG_NAME "")
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
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/ErrorReporting.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/LimeSuiteConfig.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/ADF4002/ADF4002.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/version.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/lms7002m_mcu/MCU_BD.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/lms7002m_mcu/MCU_File.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/ConnectionRegistry/IConnection.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/ConnectionRegistry/ConnectionHandle.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/ConnectionRegistry/ConnectionRegistry.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/lms7002m_novena/StreamerNovena.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/lms7002m/LMS7002M.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/lms7002m/CalibrationCache.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/lms7002m/cpp-feather-ini-parser/INI.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/lms7002m/LMS7002M_RegistersMap.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/lms7002m/LMS7002M_parameters.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/protocols/ADCUnits.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/protocols/LMS64CCommands.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/protocols/LMS64CProtocol.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/protocols/LMSBoards.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/LTEpackets/dataTypes.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/LTEpackets/fifo.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/LTEpackets/StreamerLTE.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/Si5351C/Si5351C.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/LMS_StreamBoard/LMS_StreamBoard.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/LMS_StreamBoard/LMS_StreamBoard_FIFO.h"
    "/home/srdjan/Work/LimeSuite/dpd_test2/src/FPGA_common/FPGA_common.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/srdjan/Work/LimeSuite/dpd_test2/build/libLimeSuite.a")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lms7suite" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lms7suite")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lms7suite"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/srdjan/Work/LimeSuite/dpd_test2/build/bin/lms7suite")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lms7suite" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lms7suite")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lms7suite"
         OLD_RPATH "/usr/local/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/lms7suite")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/srdjan/Work/LimeSuite/dpd_test2/build/oglGraph/cmake_install.cmake")
  include("/home/srdjan/Work/LimeSuite/dpd_test2/build/SoapyLMS7/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/srdjan/Work/LimeSuite/dpd_test2/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
