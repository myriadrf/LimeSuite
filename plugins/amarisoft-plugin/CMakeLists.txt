project("LimeSuite-amarisoft-plugin")
cmake_minimum_required(VERSION 3.00)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING INTERNAL FORCE)
set(BUILD_SHARED_LIBS "Build the shared library" ON)
set(CMAKE_BUILD_TYPE "Release")

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_AMARISOFT_PLUGIN "Build LimeSuite integration to Amarisoft" ON "UNIX;ENABLE_LIBRARY" OFF)
add_feature_info(AMARISOFT_PLUGIN ENABLE_AMARISOFT_PLUGIN "LimeSuite Amarisoft integration plugin")

if (NOT ENABLE_AMARISOFT_PLUGIN)
    return()
endif()

#include modules for finding libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#is this processor x86? set variable X86
if(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|i386.*|x86.*|amd64.*|AMD64.*")
    set(X86 TRUE)
else()
    set(X86 FALSE)
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wpedantic)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -Wl,-z,defs")

    if(NOT DEFAULT_SIMD_FLAGS)
        set(DEFAULT_SIMD_FLAGS "native")
    endif()
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr" AND X86)
        set(DEFAULT_SIMD_FLAGS "SSE3")
    endif()

    SET(ENABLE_SIMD_FLAGS "${DEFAULT_SIMD_FLAGS}" CACHE STRING "Set compiler SIMD flags")
    SET_PROPERTY(CACHE ENABLE_SIMD_FLAGS PROPERTY STRINGS none native SSE3 SSE4.2 AVX AVX2) 

    if(${ENABLE_SIMD_FLAGS} MATCHES "native")
        add_definitions(-march=native)
        if(X86)
            add_definitions(-mfpmath=both)
        endif()
        message(STATUS "Enabling native optimizations")
    elseif(${ENABLE_SIMD_FLAGS} MATCHES "AVX2")
        add_definitions(-mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt -mavx -mfma -mavx2 -mfpmath=both)
        message(STATUS "Enabling AVX2 instructions")
    elseif(${ENABLE_SIMD_FLAGS} MATCHES "AVX")
        add_definitions(-mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt -mavx -mfpmath=both)
        message(STATUS "Enabling AVX instructions")
    elseif(${ENABLE_SIMD_FLAGS} MATCHES "SSE4.2")
        add_definitions(-mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt -mfpmath=both)
        message(STATUS "Enabling SSE4.2 instructions")
    elseif(${ENABLE_SIMD_FLAGS} MATCHES "SSE3")
        add_definitions(-mmmx -msse -msse2 -msse3 -mfpmath=both)
        message(STATUS "Enabling SSE3 instructions")
    endif()
endif()

add_library(trx_limesuite trx_limesuite.cpp)
target_link_libraries(trx_limesuite LimeSuite2)

#remove lib prefix, Amarisoft searches for trx_*.so
set_target_properties(trx_limesuite PROPERTIES PREFIX "")
