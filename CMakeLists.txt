cmake_minimum_required(VERSION 3.12)
project(AudioEditor VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard and flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set explicit compiler flags for macOS compatibility
if(APPLE)
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -Wall")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create the main library
add_library(AudioEditorLib STATIC
    SoundSegment.cpp
)

target_include_directories(AudioEditorLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create the demo executable
add_executable(demo
    demo.cpp
)

target_link_libraries(demo
    AudioEditorLib
)

# Create Python bindings library (optional)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 REQUIRED)
    
    pybind11_add_module(audio_editor_python
        python_bindings.cpp
    )
    
    target_link_libraries(audio_editor_python PRIVATE
        AudioEditorLib
    )
endif()

# Testing
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    
    # Add test executables
    add_executable(test_basic
        tests/test_basic.cpp
    )
    
    target_link_libraries(test_basic
        AudioEditorLib
    )
    
    add_test(NAME BasicTests COMMAND test_basic)
endif()

# Installation
install(TARGETS AudioEditorLib demo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES SoundSegment.hpp
    DESTINATION include
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}") 