message(STATUS "")
message(STATUS "##############################################################")
message(STATUS "API Examples Enabled")
message(STATUS "##############################################################")

add_executable(basicRX 
    basicRX.cpp
)
target_link_libraries(basicRX
    LimeSuite
)

add_executable(singleRX 
    singleRX.cpp
)
target_link_libraries(singleRX
    LimeSuite
)

add_executable(dualRXTX 
    dualRXTX.cpp
)
target_link_libraries(dualRXTX
    LimeSuite
)



