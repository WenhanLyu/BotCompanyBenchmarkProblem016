cmake_minimum_required(VERSION 3.10)

# Set compiler to gcc-13/g++-13 if available (OJ environment)
# Otherwise use system default (local development)
if(NOT CMAKE_C_COMPILER)
    find_program(GCC13 gcc-13)
    if(GCC13)
        set(CMAKE_C_COMPILER gcc-13)
    endif()
endif()

if(NOT CMAKE_CXX_COMPILER)
    find_program(GPP13 g++-13)
    if(GPP13)
        set(CMAKE_CXX_COMPILER g++-13)
    endif()
endif()

project(BPlusTreeDB)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Define source files (will be populated as implementation progresses)
set(SOURCES
    main.cpp
    node.cpp
    file_manager.cpp
)

# Create executable named "code" in project root
add_executable(code ${SOURCES})

# Set output directory to project root
set_target_properties(code PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
