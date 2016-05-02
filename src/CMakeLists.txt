########################################################################
## Create lime-suite project
########################################################################
cmake_minimum_required(VERSION 3.1.3)
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

if (MSVC)
	SET( MSVC_MULTITHREAD_COMPILE_FLAGS "/MP")
	SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${MSVC_MULTITHREAD_COMPILE_FLAGS}" )
endif(MSVC)

#version numbers built into shared library
#todo version number, example could be date, standard major.minor.patch
#todo abi compatibility, could be version, date, incrementing number
#maybe parse version.h for info?
set(LIME_SUITE_VERSION 0.0.0)
set(LIME_SUITE_ABI 1)

########################################################################
## public headers
########################################################################
set(LIME_SUITE_PUBLIC_HEADERS
    API/LimeSuite.h
    lms7002m/LMS7002M_parameters.h
)

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_HEADERS "Enable install headers" ON "TRUE" OFF)
add_feature_info(LimeSuiteHeaders ENABLE_HEADERS "The lime suite headers")
if (ENABLE_HEADERS)
    install(
        FILES ${LIME_SUITE_PUBLIC_HEADERS}
        DESTINATION include/lime
    )
endif()

########################################################################
## lime suite library
########################################################################
if(WIN32)
add_subdirectory(sqlite-3.12)
include_directories(sqlite-3.12)
set (SQLITE3_LIBRARY sqlite3_static)
else()
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
find_library(SQLITE3_LIBRARY sqlite3)
if(NOT SQLITE3_INCLUDE_DIR)
    message(SEND_ERROR "Failed to find sqlite3.h, please install libsqlite3-dev")
else()
    include_directories(${SQLITE3_INCLUDE_DIR})
endif()
if(NOT SQLITE3_LIBRARY)
    message(SEND_ERROR "Failed to find the sqlite3 library, please install libsqlite3-dev")
endif()

message(STATUS "SQLITE3")

message(STATUS ${SQLITE3_INCLUDE_DIR})
message(STATUS ${SQLITE3_LIBRARY})
endif()

add_subdirectory(GFIR)

set(LIME_SUITE_SOURCES
    ErrorReporting.cpp
    ADF4002/ADF4002.cpp
    lms7002m_mcu/MCU_BD.cpp
    ConnectionRegistry/IConnection.cpp
    ConnectionRegistry/ConnectionHandle.cpp
    ConnectionRegistry/ConnectionRegistry.cpp
    lms7002m_novena/StreamerNovena.cpp
    lms7002m/LMS7002M_RegistersMap.cpp
    lms7002m/LMS7002M_parameters.cpp
    lms7002m/LMS7002M.cpp
    lms7002m/LMS7002M_RxTxCalibrations.cpp
    lms7002m/mcu_dc_iq_calibration.cpp
    lms7002m/CalibrationCache.cpp
    lms7002m/LMS7002M_filtersCalibration.cpp
    protocols/LMS64CProtocol.cpp
    LTEpackets/StreamerLTE.cpp
    Si5351C/Si5351C.cpp
    LMS_StreamBoard/LMS_StreamBoard.cpp
    kissFFT/kiss_fft.c
    API/lms7_api.cpp
    API/lms7_device.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(ADF4002)
include_directories(lms7002m_mcu)
include_directories(ConnectionRegistry)
include_directories(lms7002m_novena)
include_directories(lms7002m)
include_directories(lms7002m/cpp-feather-ini-parser)
include_directories(HPM7)
include_directories(protocols)
include_directories(LTEpackets)
include_directories(Si5351C)
include_directories(LMS_StreamBoard)
include_directories(kissFFT)
include_directories(API)
include_directories(GFIR)

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIBRARY "Enable build library" ON "TRUE" OFF)
add_feature_info(LimeSuiteLibrary ENABLE_LIBRARY "The lime suite library")
if (ENABLE_LIBRARY)
    add_library(LimeSuite SHARED ${LIME_SUITE_SOURCES})
    target_link_libraries(LimeSuite ${SQLITE3_LIBRARY} lms_gfir)
    set_property(TARGET LimeSuite PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    set_target_properties(LimeSuite PROPERTIES SOVERSION ${LIME_SUITE_ABI})
    set_target_properties(LimeSuite PROPERTIES VERSION ${LIME_SUITE_VERSION})
    set_target_properties(LimeSuite PROPERTIES DEFINE_SYMBOL "LIME_DLL")

    install(TARGETS LimeSuite
        LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
        ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
        RUNTIME DESTINATION bin              # .dll file
    )

    set(LIME_SUITE_LIBS LimeSuite)
endif()

########################################################################
## Supported connections
########################################################################
include(ConnectionEVB7COM/CMakeLists.txt)
include(ConnectionSTREAM/CMakeLists.txt)
include(ConnectionNovenaRF7/CMakeLists.txt)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionRegistry/BuiltinConnections.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/BuiltinConnections.cpp
)

target_sources(LimeSuite PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/BuiltinConnections.cpp)

########################################################################
## wx widgets dependency
########################################################################
set(wxWidgets_USE_DEBUG OFF)
set(wxWidgets_USE_UNICODE ON)
set(wxWidgets_USE_UNIVERSAL OFF)
set(wxWidgets_USE_STATIC ON)
find_package(wxWidgets COMPONENTS richtext xml html xrc adv aui core base gl)
message(STATUS "wxWidgets_DEFINITIONS: ${wxWidgets_DEFINITIONS}")
message(STATUS "wxWidgets_INCLUDE_DIRS: ${wxWidgets_INCLUDE_DIRS}")
message(STATUS "wxWidgets_LIBRARIES: ${wxWidgets_LIBRARIES}")

########################################################################
## lms7suite gui app
########################################################################
set(LMS7SUITE_GUI_SOURCES
    ADF4002/ADF4002_wxgui.cpp
    lms7002_wxgui/lms7002_pnlBuffers_view.cpp
    lms7002_wxgui/lms7002_pnlXBUF_view.cpp
    lms7002_wxgui/lms7002_wxgui.cpp
    lms7002_wxgui/lms7002_pnlRFE_view.cpp
    lms7002_wxgui/lms7002_pnlCalibrations_view.cpp
    lms7002_wxgui/lms7002_pnlTRF_view.cpp
    lms7002_wxgui/lms7002_pnlCDS_view.cpp
    lms7002_wxgui/lms7002_pnlLimeLightPAD_view.cpp
    lms7002_wxgui/lms7002_pnlBIST_view.cpp
    lms7002_wxgui/dlgViewIRAM.cpp
    lms7002_wxgui/lms7002_gui_utilities.cpp
    lms7002_wxgui/lms7002_pnlSX_view.cpp
    lms7002_wxgui/lms7002_pnlTBB_view.cpp
    lms7002_wxgui/lms7002_dlgVCOfrequencies.cpp
    lms7002_wxgui/lms7002_pnlRBB_view.cpp
    lms7002_wxgui/lms7002_pnlLDO_view.cpp
    lms7002_wxgui/lms7002_pnlAFE_view.cpp
    lms7002_wxgui/CoefficientFileParser.cpp
    lms7002_wxgui/lms7002_pnlRxTSP_view.cpp
    lms7002_wxgui/lms7002_mainPanel.cpp
    lms7002_wxgui/pnlBoardControls.cpp
    lms7002_wxgui/dlgViewSFR.cpp
    lms7002_wxgui/lms7002_pnlCLKGEN_view.cpp
    lms7002_wxgui/lms7002_pnlMCU_BD_view.cpp
    lms7002_wxgui/lms7002_pnlBIAS_view.cpp
    lms7002_wxgui/lms7002_dlgGFIR_Coefficients.cpp
    lms7002_wxgui/lms7002_pnlTxTSP_view.cpp
    LMS_Programing/LMS_Programing_wxgui.cpp
    utilities_gui/dlgConnectionSettings.cpp
    utilities_gui/pnlMiniLog.cpp
    utilities_gui/SPI_wxgui.cpp
    utilities_gui/dlgFullMessageLog.cpp
    utilities_gui/dlgDeviceInfo.cpp
    utilities_gui/utilities_gui.cpp
    lms7suiteAppFrame.cpp
    lms7002m_novena/lms7002m_novena_wxgui.cpp
    RFSpark/RFSpark_wxgui.cpp
    Si5351C/Si5351C_wxgui.cpp
    lms7suiteApp_gui.cpp
    lms7suiteApp.cpp
    FPGAcontrols_wxgui/FPGAcontrols_wxgui.cpp
    HPM7/HPM7_wxgui.cpp
    dlgAbout.cpp
    numericSlider/numericSlider.cpp
    myriad7/myriad7_wxgui.cpp
    fftviewer_wxgui/fftviewer_wxgui.cpp
    fftviewer_wxgui/fftviewer_frFFTviewer.cpp
    lms7suiteEvents/lms7suiteEvents.cpp
)

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LMS7_GUI "Enable build lms7suite" ON "ENABLE_LIBRARY;wxWidgets_FOUND" OFF)
add_feature_info(LimeSuiteGUILMS7 ENABLE_LMS7_GUI "GUI Application for LMS7")
if (ENABLE_LMS7_GUI)

    include_directories(lms7002_wxgui)
    include_directories(LMS_Programing)
    include_directories(utilities_gui)
    include_directories(RFSpark)
    include_directories(FPGAcontrols_wxgui)
    include_directories(numericSlider)
    include_directories(myriad7)
    include_directories(fftviewer_wxgui)
    include_directories(lms7suiteEvents)
    include(${wxWidgets_USE_FILE})
    add_subdirectory(oglGraph)

    set(BINARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")

    add_executable(lms7suite ${LMS7SUITE_GUI_SOURCES} resource.rc)
    set_target_properties(lms7suite PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
    target_link_libraries(lms7suite ${LIME_SUITE_LIBS} oglGraph ${wxWidgets_LIBRARIES})

    if (MSVC)
        set_property(TARGET lms7suite APPEND PROPERTY LINK_FLAGS /SUBSYSTEM:WINDOWS)
    endif(MSVC)

    install(TARGETS lms7suite DESTINATION bin)

endif()

########################################################################
## update version.h
########################################################################
set(AUTO_UPDATE_VERSION OFF CACHE BOOL "Uses python script to increment version")
if(AUTO_UPDATE_VERSION)
	add_custom_target(updateVersion ALL COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/auto_version.py WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	add_dependencies(lms7suite updateVersion)
endif()

########################################################################
# Lime Util -- command line utility for quick testing connections
########################################################################
add_executable(LimeUtil LimeUtil.cpp)
target_link_libraries(LimeUtil ${LIME_SUITE_LIBS})

########################################################################
# boardEmulator -- creates serial port and imitates board communications
########################################################################
if(UNIX)
    add_executable(boardEmulator boardEmulator.cpp)
endif()

########################################################################
## SoapySDR bindings for LMS7 RFIC driver
########################################################################
add_subdirectory(SoapyLMS7)

########################################################################
## udev rules for linux usb
########################################################################
if (DEFINED UDEV_RULES_PATH)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/../udev-rules/64-limesuite.rules
        DESTINATION ${UDEV_RULES_PATH})
endif()


#########################################################################
# summary
#########################################################################
include(FeatureSummary)
message(STATUS "")
message(STATUS "######################################################")
message(STATUS "## ${PROJECT_NAME} enabled features")
message(STATUS "######################################################")
feature_summary(WHAT ENABLED_FEATURES)
message(STATUS "######################################################")
message(STATUS "## ${PROJECT_NAME} disabled features")
message(STATUS "######################################################")
feature_summary(WHAT DISABLED_FEATURES)
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Lime Suite Version: ${LIME_SUITE_VERSION}")
message(STATUS "Lime Suite ABI: ${LIME_SUITE_ABI}")
