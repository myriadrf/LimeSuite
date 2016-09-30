########################################################################
## public headers
########################################################################
set(LIME_SUITE_PUBLIC_HEADERS
    lime/LimeSuite.h
    VersionInfo.h
    ErrorReporting.h
    LimeSuiteConfig.h
    ADF4002/ADF4002.h
    version.h
    lms7002m_mcu/MCU_BD.h
    lms7002m_mcu/MCU_File.h
    ConnectionRegistry/IConnection.h
    ConnectionRegistry/ConnectionHandle.h
    ConnectionRegistry/ConnectionRegistry.h
    lms7002m/LMS7002M.h
    lms7002m/LMS7002M_RegistersMap.h
    lms7002m/LMS7002M_parameters.h
    protocols/ADCUnits.h
    protocols/LMS64CCommands.h
    protocols/LMS64CProtocol.h
    protocols/ILimeSDRStreaming.cpp
    protocols/LMSBoards.h
    protocols/dataTypes.h
    protocols/fifo.h
    Si5351C/Si5351C.h
    FPGA_common/FPGA_common.h
    lime/LimeSuite.h
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
set(LIME_SUITE_SOURCES
    ErrorReporting.cpp
    ADF4002/ADF4002.cpp
    lms7002m_mcu/MCU_BD.cpp
    ConnectionRegistry/IConnection.cpp
    ConnectionRegistry/ConnectionHandle.cpp
    ConnectionRegistry/ConnectionRegistry.cpp
    lms7002m/LMS7002M_RegistersMap.cpp
    lms7002m/LMS7002M_parameters.cpp
    lms7002m/LMS7002M.cpp
    lms7002m/LMS7002M_RxTxCalibrations.cpp
    lms7002m/goert.cpp
    lms7002m/mcu_dc_iq_calibration.cpp
    lms7002m/CalibrationCache.cpp
    lms7002m/LMS7002M_filtersCalibration.cpp
    protocols/LMS64CProtocol.cpp
    protocols/ILimeSDRStreaming.cpp
    Si5351C/Si5351C.cpp
    kissFFT/kiss_fft.c
    API/lms7_api.cpp
    API/lms7_device.cpp
    FPGA_common/FPGA_common.cpp
    windowFunction.cpp
)

set(LIME_SUITE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    API
    GFIR
    protocols
    ConnectionRegistry
    lms7002m_mcu
    ADF4002
    Si5351C
    lms7002m
    LTEpackets
    FPGA_common
    lms7002m_mcu
    ${PROJECT_SOURCE_DIR}/external/cpp-feather-ini-parser
    HPM7
    kissFFT
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/VersionInfo.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.cpp @ONLY)
list(APPEND LIME_SUITE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.cpp)

if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND LIME_SUITE_LIBRARIES -pthread)
endif(CMAKE_COMPILER_IS_GNUCXX)

#sqlite depedency
list(APPEND LIME_SUITE_INCLUDES ${SQLITE3_INCLUDE_DIRS})
list(APPEND LIME_SUITE_LIBRARIES ${SQLITE3_LIBRARIES})

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIBRARY "Enable build library" ON "SQLITE3_FOUND" OFF)
add_feature_info(LimeSuiteLibrary ENABLE_LIBRARY "The lime suite library")
if (ENABLE_LIBRARY)

    add_library(LimeSuite SHARED ${LIME_SUITE_SOURCES})
    target_link_libraries(LimeSuite ${LIME_SUITE_LIBRARIES})
    set_property(TARGET LimeSuite PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    set_target_properties(LimeSuite PROPERTIES SOVERSION ${LIME_SUITE_SOVER})
    set_target_properties(LimeSuite PROPERTIES VERSION ${LIME_SUITE_LIBVER})
    set_target_properties(LimeSuite PROPERTIES DEFINE_SYMBOL "LIME_DLL")
    target_include_directories(LimeSuite PUBLIC ${LIME_SUITE_INCLUDES})

    install(TARGETS LimeSuite
        LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
        ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
        RUNTIME DESTINATION bin              # .dll file
    )
	
	if (MSVC)
		set_target_properties(LimeSuite PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
	endif(MSVC)

    add_subdirectory(GFIR)
    set(LIME_SUITE_LIBS LimeSuite)
endif()

########################################################################
## Supported connections
########################################################################
include(ConnectionEVB7COM/CMakeLists.txt)
include(ConnectionSTREAM/CMakeLists.txt)
include(ConnectionSTREAM_UNITE/CMakeLists.txt)
include(ConnectionNovenaRF7/CMakeLists.txt)
include(Connection_uLimeSDR/CMakeLists.txt)
include(ConnectionXillybus/CMakeLists.txt)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionRegistry/BuiltinConnections.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/BuiltinConnections.cpp
)

target_sources(LimeSuite PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/BuiltinConnections.cpp)

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
    boards_wxgui/lms7002m_novena_wxgui.cpp
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
    boards_wxgui/pnlQSpark.cpp
    boards_wxgui/pnluLimeSDR.cpp
    kissFFT/kiss_fft.c
    windowFunction.cpp
    boards_wxgui/pnlLimeSDR.cpp
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
    include_directories(boards_wxgui)
    include(${wxWidgets_USE_FILE})
    add_subdirectory(oglGraph)

    set(BINARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")

    add_executable(lms7suite ${LMS7SUITE_GUI_SOURCES} resources/resource.rc)
    set_target_properties(lms7suite PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
    target_link_libraries(lms7suite LimeSuite oglGraph ${wxWidgets_LIBRARIES})

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
# boardEmulator -- creates serial port and imitates board communications
########################################################################
if(UNIX)
    add_executable(boardEmulator boardEmulator.cpp)
    target_link_libraries(boardEmulator LimeSuite)
endif()

#########################################################################
# utilities
#########################################################################
set(BUILD_UTILITIES OFF CACHE BOOL "Additional utility programs for testing")
if(BUILD_UTILITIES)
    include_directories(utilityTools)
    set(utility_src_files utilityTools/pllTest.cpp)
    add_executable(utilityTools ${utility_src_files} utilityTools/main.cpp)
    target_link_libraries(utilityTools LimeSuite)
endif()

########################################################################
# Lime Util -- command line utility, info and connections
########################################################################
if(BUILD_UTILITIES)
	add_executable(LimeUtil LimeUtil.cpp)
	target_link_libraries(LimeUtil LimeSuite)
	install(TARGETS LimeUtil DESTINATION bin)
endif()

#########################################################################
# tests
#########################################################################
set(BUILD_TESTS OFF CACHE BOOL "Download gtest and build test application")
if(BUILD_TESTS)
    add_executable(RxTest utilityTools/RxTest.cpp)
    add_executable(RxTest2 utilityTools/RxTest2.cpp)
    target_link_libraries(RxTest LimeSuite)
    target_link_libraries(RxTest2 LimeSuite)
    add_subdirectory(tests)
endif()

set(BUILD_EXAMPLES ON CACHE BOOL "API Examples")
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

