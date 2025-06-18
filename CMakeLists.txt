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

if(UNIX)
    add_custom_command(TARGET GameEngine
            POST_BUILD
            # 1) Tell me you’re in the UNIX branch
            COMMAND ${CMAKE_COMMAND} -E echo
            "=== [Shader] UNIX: running shadercompile.sh in ${CMAKE_SOURCE_DIR} ==="

            # 2) Actually run it (you need bash on your PATH)
            COMMAND bash "${CMAKE_SOURCE_DIR}/shadercompile.sh"

            # 3) Then copy shaders
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:GameEngine>/shaders"

            COMMENT "Post-build: compile shaders & copy on UNIX"
    )
elseif(WIN32)
    add_custom_command(TARGET GameEngine
            POST_BUILD
            # Make sure we run *from* your source dir
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"

            # 1) Announce that we're about to run the .bat
            COMMAND ${CMAKE_COMMAND} -E echo
            "=== [Shader] WINDOWS: invoking shadercompile.bat in ${CMAKE_SOURCE_DIR} ==="

            # 2) Actually run it
            COMMAND cmd.exe /C shadercompile.bat

            # 3) Announce that we're about to copy
            COMMAND ${CMAKE_COMMAND} -E echo
            "=== [Shader] WINDOWS: copying shaders to $<TARGET_FILE_DIR:GameEngine>/shaders ==="

            # 4) Copy the folder
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:GameEngine>/shaders"

            COMMENT "Post-build: compile shaders & copy assets"
    )
endif()