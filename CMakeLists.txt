cmake_minimum_required(VERSION 3.31)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL3 EXCLUDE_FROM_ALL)

set(VULKAN_SDK $ENV{VULKAN_SDK})
include_directories(${VULKAN_SDK}/Include)
link_directories(${VULKAN_SDK}/Lib)

add_executable(GameEngine
    main.cpp
    common.cpp
    utility.cpp
)
target_link_libraries(GameEngine PRIVATE SDL3::SDL3)

find_package(Vulkan REQUIRED)

target_link_libraries(GameEngine PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
)