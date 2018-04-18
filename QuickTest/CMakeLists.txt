
#find FLTK
set(FLTK_SKIP_FLUID true)
find_package(FLTK)
execute_process(COMMAND fltk-config --ldflags OUTPUT_VARIABLE FLTK_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)

cmake_dependent_option(ENABLE_QUICKTEST "LimeSDR quick test utility" ON "ENABLE_LIBRARY;FLTK_FOUND" OFF)
add_feature_info(LimeQuickTest ENABLE_QUICKTEST "LimeSDR-QuickTest Utility")

if(ENABLE_QUICKTEST)
    include_directories(${FLTK_INCLUDE_DIR})
    add_executable(LimeQuickTest LimeSDRTest.cpp TestGUI.cpp resources/resource.rc  ${PROJECT_SOURCE_DIR}/external/kissFFT/kiss_fft.c)
    target_link_libraries(LimeQuickTest
		      LimeSuite
		      ${FLTK_LDFLAGS}
		      ${FLTK_LIBRARIES}
		      ${GLUT_LIBRARY}
		      )
    install(TARGETS LimeQuickTest DESTINATION bin)
endif()





