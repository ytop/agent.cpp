# cmake/UbuntuGcc.cmake
# Enforces an Ubuntu Linux + GNU gcc toolchain.
# Halts configuration before any source file is compiled on unsupported platforms.

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR
        "Pie_Cpp only supports Ubuntu Linux. Detected system: ${CMAKE_SYSTEM_NAME}.")
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR
        "Pie_Cpp requires the GNU gcc/g++ toolchain on Ubuntu, got: ${CMAKE_CXX_COMPILER_ID}.\n"
        "Install via: sudo apt install g++")
endif()

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13")
    message(FATAL_ERROR
        "Pie_Cpp requires gcc 13 or later (Ubuntu 24.04 default), got: ${CMAKE_CXX_COMPILER_VERSION}.\n"
        "Install via: sudo apt install g++")
endif()
