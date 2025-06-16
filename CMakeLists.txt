cmake_minimum_required(VERSION 3.31)
project(GameEngine)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL3 EXCLUDE_FROM_ALL)

add_executable(GameEngine main.cpp
        common.cpp
        common.h
        utility.cpp
        utility.h)
target_link_libraries(GameEngine PRIVATE SDL3::SDL3)
add_custom_command(TARGET GameEngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:GameEngine>/shaders"
)