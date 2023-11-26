set(MAIN_LIBRARY_NAME LimeSuite2)
set(GUI_EXECUTABLE_NAME LimeSuiteGUI2)

########################################################################
## public headers
########################################################################
FILE(GLOB LIME_SUITE_PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/limesuite/*.h")

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_HEADERS "Enable install headers" ON "TRUE" OFF)
add_feature_info(LimeSuiteHeaders ENABLE_HEADERS "The lime suite headers")
if (ENABLE_HEADERS)
    install(
        FILES ${LIME_SUITE_PUBLIC_HEADERS}
        DESTINATION include/limesuite
    )
endif()

########################################################################
## lime suite library
########################################################################
set(LIME_SUITE_SOURCES
    SDRDevice.cpp
    StreamComposite.cpp
    CommonFunctions.cpp
    boards/DeviceRegistry.cpp
    boards/DeviceHandle.cpp
    boards/LMS7002M_SDRDevice.cpp
    Logger.cpp
    ADF4002/ADF4002.cpp
    lms7002m/MCU_BD.cpp
    lms7002m/MCU_File.cpp
    lms7002m/LMS7002M_RegistersMap.cpp
    lms7002m/LMS7002M_parameters.cpp
    lms7002m/LMS7002M.cpp
    lms7002m/LMS7002M_validation.cpp
    lms7002m/LMS7002M_RxTxCalibrations.cpp
    lms7002m/LMS7002M_BaseCalibrations.cpp
    lms7002m/mcu_dc_iq_calibration.cpp
    lms7002m/LMS7002M_filtersCalibration.cpp
    lms7002m/LMS7002M_gainCalibrations.cpp
    protocols/LMS64CProtocol.cpp
    protocols/TRXLooper.cpp
    protocols/BufferInterleaving.cpp
    Si5351C/Si5351C.cpp
    ${PROJECT_SOURCE_DIR}/external/kissFFT/kiss_fft.c
    FPGA_common/FPGA_common.cpp
    windowFunction.cpp
    threadHelper/threadHelper.cpp
    CDCM6208/CDCM6208_Dev.cpp
    Profiler.cpp
    gnuPlotPipe.h

    DSP/Equalizer.cpp
    memory/MemoryPool.cpp

    mcu_program/spi.cpp
    mcu_program/common_src/lms7002m_calibrations.c
    mcu_program/common_src/lms7002m_controls.c
    mcu_program/common_src/lms7002m_filters.c
)

SET_SOURCE_FILES_PROPERTIES( 
    mcu_program/common_src/lms7002m_calibrations.c
    mcu_program/common_src/lms7002m_controls.c
    mcu_program/common_src/lms7002m_filters.c
    PROPERTIES LANGUAGE CXX)

set(LIME_SUITE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/boards
    ${CMAKE_CURRENT_SOURCE_DIR}/protocols
    ${CMAKE_CURRENT_SOURCE_DIR}/lms7002m
    ${CMAKE_CURRENT_SOURCE_DIR}/FPGA_common
    ${PROJECT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/threadHelper
    ${CMAKE_CURRENT_SOURCE_DIR}/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/vectorization
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/VersionInfo.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.cpp @ONLY)
list(APPEND LIME_SUITE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.cpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/SystemResources.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/SystemResources.cpp @ONLY)
list(APPEND LIME_SUITE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/SystemResources.cpp)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
list(APPEND LIME_SUITE_LIBRARIES Threads::Threads)

include(CheckAtomic)
if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB OR NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
    list(APPEND LIME_SUITE_LIBRARIES atomic)
endif()

include(FeatureSummary)
include(CMakeDependentOption)

add_subdirectory(vectorization)

option(ENABLE_LIBRARY "Enable build library" ON)
add_feature_info(LimeSuiteLibrary ENABLE_LIBRARY "The lime suite library")
if (ENABLE_LIBRARY)
    add_library(${MAIN_LIBRARY_NAME} ${LIME_SUITE_SOURCES})
    target_link_libraries(${MAIN_LIBRARY_NAME} PUBLIC ${LIME_SUITE_LIBRARIES} samplesConversion)
    set_property(TARGET ${MAIN_LIBRARY_NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    set_target_properties(${MAIN_LIBRARY_NAME} PROPERTIES SOVERSION ${LIME_SUITE_SOVER})
    set_target_properties(${MAIN_LIBRARY_NAME} PROPERTIES VERSION ${LIME_SUITE_LIBVER})
    set_target_properties(${MAIN_LIBRARY_NAME} PROPERTIES DEFINE_SYMBOL "LIME_DLL_EXPORTS")
    target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE ${LIME_SUITE_INCLUDES})
    target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${MAIN_LIBRARY_NAME} PUBLIC "LIME_DLL")
    endif()

    install(TARGETS ${MAIN_LIBRARY_NAME}
        LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
        ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
        RUNTIME DESTINATION bin              # .dll file
    )

	if (MSVC)
		set_target_properties(${MAIN_LIBRARY_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
	endif(MSVC)

    add_subdirectory(GFIR)
    set(LIME_SUITE_LIBS LimeSuite)
endif()

########################################################################
## Supported connections
########################################################################
include(comms/USB/CMakeLists.txt)
include(comms/PCIe/CMakeLists.txt)

include(boards/LimeSDR/CMakeLists.txt)
include(boards/LimeSDR_X3/CMakeLists.txt)
include(boards/LimeSDR_XTRX/CMakeLists.txt)
include(boards/MMX8/CMakeLists.txt)
include(boards/LimeSDR_Mini/CMakeLists.txt)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/BoardSupportLoader.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/BoardSupportLoader.cpp
)

target_sources(${MAIN_LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/BoardSupportLoader.cpp)

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
## LimeSuite GUI app
########################################################################
set(LMS7SUITE_GUI_SOURCES
    GUI/ILMS7002MTab.cpp
    ADF4002/ADF4002_wxgui.cpp
    CDCM6208/CDCM6208_panelgui.cpp
    lms7002_wxgui/SDRConfiguration_view.cpp
    lms7002_wxgui/lms7002_pnlXBUF_view.cpp
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
    lms7002_wxgui/lms7002_pnlGains_view.cpp
    lms7002_wxgui/CoefficientFileParser.cpp
    lms7002_wxgui/lms7002_pnlRxTSP_view.cpp
    lms7002_wxgui/lms7002_mainPanel.cpp
    # lms7002_wxgui/dlgViewSFR.cpp
    lms7002_wxgui/lms7002_pnlCLKGEN_view.cpp
    lms7002_wxgui/lms7002_pnlMCU_BD_view.cpp
    lms7002_wxgui/lms7002_pnlBIAS_view.cpp
    lms7002_wxgui/lms7002_dlgGFIR_Coefficients.cpp
    lms7002_wxgui/lms7002_pnlTxTSP_view.cpp
    lms7002_wxgui/lms7002_pnlR3.cpp
    LMS_Programing/LMS_Programing_wxgui.cpp
    utilities_gui/pnlMiniLog.cpp
    # utilities_gui/pnlAPI.cpp
    utilities_gui/SPI_wxgui.cpp
    utilities_gui/dlgFullMessageLog.cpp
    # utilities_gui/dlgDeviceInfo.cpp
    utilities_gui/utilities_gui.cpp
    lms7suiteAppFrame.cpp
    #boards_wxgui/RFSpark_wxgui.cpp
    Si5351C/Si5351C_wxgui.cpp
    lms7suiteApp_gui.cpp
    lms7suiteApp.cpp
    #FPGAcontrols_wxgui/FPGAcontrols_wxgui.cpp
    dlgAbout.cpp
    numericSlider/numericSlider.cpp
    fftviewer_wxgui/fftviewer_wxgui.cpp
    fftviewer_wxgui/fftviewer_frFFTviewer.cpp
    lms7suiteEvents/lms7suiteEvents.cpp
    boards_wxgui/pnlBoardControls.cpp
    boards_wxgui/pnlX3.cpp
    boards_wxgui/pnlXTRX.cpp
    # boards_wxgui/pnlQSpark.cpp
    boards_wxgui/pnluLimeSDR.cpp
    # boards_wxgui/pnlUltimateEVB.cpp
    # boards_wxgui/pnlBuffers.cpp
    # boards_wxgui/pnlCoreSDR.cpp
    # boards_wxgui/pnlLimeNetMicro.cpp
    ${PROJECT_SOURCE_DIR}/external/kissFFT/kiss_fft.c
    windowFunction.cpp
    boards_wxgui/pnlLimeSDR.cpp
    boards_wxgui/pnlGPIO.cpp
    GUI/CommsToDevice.cpp
    GUI/DeviceConnectionPanel.cpp
    GUI/events.cpp
)

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_GUI "Enable build GUI" ON "ENABLE_LIBRARY;wxWidgets_FOUND" OFF)
add_feature_info(${GUI_EXECUTABLE_NAME} ENABLE_GUI "GUI Application for LimeSuite")
if (ENABLE_GUI)

    include_directories(lms7002_wxgui)
    include_directories(utilities_gui)
    # include_directories(RFSpark)
    include_directories(FPGAcontrols_wxgui)
    include_directories(numericSlider)
    include_directories(fftviewer_wxgui)
    include_directories(lms7suiteEvents)
    include_directories(boards_wxgui)
    #include_directories(limeRFE)
    include_directories(GUI)

    include(${wxWidgets_USE_FILE})
    add_subdirectory(oglGraph)

    set(BINARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")

    add_executable(${GUI_EXECUTABLE_NAME} ${LMS7SUITE_GUI_SOURCES} resources/resource.rc)
    set_target_properties(${GUI_EXECUTABLE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
    target_compile_definitions(${GUI_EXECUTABLE_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
    target_include_directories(${GUI_EXECUTABLE_NAME} PRIVATE ${LIME_SUITE_INCLUDES})

    target_link_libraries(${GUI_EXECUTABLE_NAME} ${MAIN_LIBRARY_NAME} oglGraph ${wxWidgets_LIBRARIES})

    if (MSVC)
        set_property(TARGET ${GUI_EXECUTABLE_NAME} APPEND PROPERTY LINK_FLAGS /SUBSYSTEM:WINDOWS)
    endif(MSVC)

    install(TARGETS ${GUI_EXECUTABLE_NAME} DESTINATION bin)

endif()

########################################################################
# Build pkg config file
########################################################################
if (ENABLE_HEADERS AND ENABLE_LIBRARY)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/LimeSuite.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/LimeSuite.pc
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/LimeSuite.pc
    DESTINATION lib${LIB_SUFFIX}/pkgconfig
)
endif (ENABLE_HEADERS AND ENABLE_LIBRARY)

#########################################################################
# utilities
#########################################################################
add_subdirectory(utilities)

#########################################################################
# examples
#########################################################################
add_subdirectory(examples)

#########################################################################
# LimeRFE
#########################################################################
# add_subdirectory(limeRFE)

#########################################################################
# Lime command line tools
#########################################################################
add_subdirectory(cli)
