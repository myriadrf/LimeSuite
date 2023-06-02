include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_CLI "Enable comman line programs" ON "ENABLE_LIBRARY" ON)
add_feature_info(LimeSuiteCLI ENABLE_CLI "LimeSuite command line tools")
if (NOT ENABLE_CLI)
    return()
endif()

# add_executable(basicRX basicRX.cpp)
# set_target_properties(basicRX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# target_link_libraries(basicRX LimeSuite)

# add_executable(singleRX singleRX.cpp)
# set_target_properties(singleRX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# target_link_libraries(singleRX LimeSuite)

# add_executable(singleRX_cpp singleRX_cpp.cpp)
# set_target_properties(singleRX_cpp PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# target_link_libraries(singleRX_cpp LimeSuite)

add_executable(LimeDevice LimeDevice.cpp)
set_target_properties(LimeDevice PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(LimeDevice PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(LimeDevice PUBLIC LimeSuite2 ${MAIN_LIBRARY_NAME})

add_executable(LimeSPI LimeSPI.cpp)
set_target_properties(LimeSPI PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_include_directories(LimeSPI PUBLIC ${LIME_SUITE_INCLUDES})
target_link_libraries(LimeSPI PUBLIC LimeSuite2 ${MAIN_LIBRARY_NAME})

# add_executable(dualRXTX dualRXTX.cpp)
# set_target_properties(dualRXTX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# target_link_libraries(dualRXTX LimeSuite)

# add_executable(gpio_example gpio_example.cpp)
# set_target_properties(gpio_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# target_link_libraries(gpio_example LimeSuite)

# add_executable(basicTX basicTX.cpp)
# set_target_properties(basicTX PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# target_link_libraries(basicTX LimeSuite)

# if(ENABLE_LIMERFE)
#     add_executable(limeRFE_USB_example limeRFE_USB_example.cpp)
#     set_target_properties(limeRFE_USB_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#     target_link_libraries(limeRFE_USB_example LimeSuite)

#     add_executable(limeRFE_I2C_example limeRFE_I2C_example.cpp)
#     set_target_properties(limeRFE_I2C_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#     target_link_libraries(limeRFE_I2C_example LimeSuite)

#     add_executable(limeRFE_ADC_example limeRFE_ADC_example.cpp)
#     set_target_properties(limeRFE_ADC_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
#     target_link_libraries(limeRFE_ADC_example LimeSuite)
# endif()