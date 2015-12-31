########################################################################
## Support for EVB7 over COM serial connection
########################################################################

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_EVB7COM "Enable EVB + COM" ON "TRUE" OFF)
add_feature_info(ConnectionEVB7COM ENABLE_EVB7COM "EVB + COM Connection support")
if (NOT ENABLE_EVB7COM)
    return()
endif()

########################################################################
## Build library
########################################################################
add_library(ConnectionEVB7COM STATIC ConnectionEVB7COMEntry.cpp ConnectionEVB7COM.cpp)
target_link_libraries(ConnectionEVB7COM connectionRegistry)
set_property(TARGET ConnectionEVB7COM PROPERTY POSITION_INDEPENDENT_CODE TRUE)
