cmake_minimum_required(VERSION 3.31)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL3 EXCLUDE_FROM_ALL)

set(VULKAN_SDK $ENV{VULKAN_SDK})
include_directories(${VULKAN_SDK}/Include)
link_directories(${VULKAN_SDK}/Lib)
find_package(Vulkan REQUIRED)

add_executable(GameEngine
    main.cpp
    common.cpp
    utility.cpp
)

target_link_libraries(GameEngine PRIVATE SDL3::SDL3 Vulkan::Vulkan)

set(SHADERS_SRC_DIR   ${CMAKE_SOURCE_DIR}/shaders)
set(SHADERS_BUILD_DIR $<TARGET_FILE_DIR:GameEngine>/shaders)  # beside the executable

add_custom_command(TARGET GameEngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling shaders with user script..."
        COMMAND ${CMAKE_COMMAND} -E env bash ${CMAKE_SOURCE_DIR}/shadercompile.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

        COMMAND ${CMAKE_COMMAND} -E echo "Staging compiled shaders to runtime directory..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SHADERS_SRC_DIR}/compiled
        ${SHADERS_BUILD_DIR}
)