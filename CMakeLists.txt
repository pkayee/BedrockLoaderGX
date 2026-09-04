cmake_minimum_required(VERSION 3.20)
set(ENV{OO_PS4_TOOLCHAIN} "${CMAKE_CURRENT_SOURCE_DIR}/third_party/PS4Toolchain")
project(BedrockLoaderGX CXX)

add_library(BedrockLoader INTERFACE)

target_include_directories(BedrockLoader
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/common/>
        $<INSTALL_INTERFACE:libraries>
        $<INSTALL_INTERFACE:res>
)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")


if(NOT DEFINED ENV{OO_PS4_TOOLCHAIN})
    message(FATAL_ERROR "OO_PS4_TOOLCHAIN environment variable is not set")
endif()

add_orbis_target(BedrockLoaderGX
    TYPE GHPLUGIN
    SOURCES ${SRC_FILES}
)

target_link_libraries(BedrockLoaderGX
        PRIVATE
        BedrockLoader
)
