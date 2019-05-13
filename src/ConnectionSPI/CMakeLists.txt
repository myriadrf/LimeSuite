########################################################################
## Support for Rasp Pi 3 SPI connection
########################################################################
set(THIS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionSPI)

set(CONNECTION_SPI_SOURCES
    ${THIS_SOURCE_DIR}/ConnectionSPIEntry.cpp
    ${THIS_SOURCE_DIR}/ConnectionSPI.cpp
)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_SPI "Enable SPI connection" OFF "ENABLE_LIBRARY" OFF)
add_feature_info(ConnectionSPI ENABLE_SPI "Rasp Pi 3 SPI Connection support")
if (NOT ENABLE_SPI)
    return()
endif()

########################################################################
## Add to library
########################################################################
target_sources(LimeSuite PRIVATE ${CONNECTION_SPI_SOURCES})
target_link_libraries(LimeSuite wiringPi crypt)

