message(STATUS "")
message(STATUS "##############################################################")
message(STATUS "API Examples Enabled")
message(STATUS "##############################################################")

if (UNIX)
find_package(Gnuplot)
if (GNUPLOT_FOUND)
add_definitions(-DUSE_GNU_PLOT)
message(STATUS "GNUPlot Found")
endif()
endif()

add_executable(basicRX basicRX.cpp)
set_target_properties(basicRX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_link_libraries(basicRX LimeSuite)

add_executable(singleRX singleRX.cpp)
set_target_properties(singleRX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_link_libraries(singleRX LimeSuite)

add_executable(dualRXTX dualRXTX.cpp)
set_target_properties(dualRXTX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_link_libraries(dualRXTX LimeSuite)



