# cmake/ApplePinClang.cmake
# Enforces Apple clang version 17.0.0 on macOS (Req 1.11).
# Halts the build before any source file is compiled if the wrong version is detected.

if(APPLE)
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --version
        OUTPUT_VARIABLE _clang_version_out
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _clang_version_rc
    )

    if(NOT _clang_version_rc EQUAL 0)
        message(FATAL_ERROR
            "Pie_Cpp: failed to run '${CMAKE_CXX_COMPILER} --version' (exit code: ${_clang_version_rc}).\n"
            "Ensure Apple clang 17.0.0 is installed via: xcode-select --install (Xcode 16.x)")
    endif()

    string(REGEX MATCH "Apple clang version ([0-9]+\\.[0-9]+\\.[0-9]+)"
           _clang_version_match "${_clang_version_out}")

    if(NOT _clang_version_match)
        message(FATAL_ERROR
            "Pie_Cpp requires Apple clang version 17.0.0 on macOS.\n"
            "Could not detect Apple clang version from compiler output.\n"
            "Detected output: ${_clang_version_out}\n"
            "Install via: xcode-select --install (Xcode 16.x)")
    endif()

    if(NOT CMAKE_MATCH_1 STREQUAL "17.0.0")
        message(FATAL_ERROR
            "Pie_Cpp requires Apple clang version 17.0.0 on macOS, got: ${CMAKE_MATCH_1}.\n"
            "Detected output: ${_clang_version_out}\n"
            "Install via: xcode-select --install (Xcode 16.x)")
    endif()
endif()
