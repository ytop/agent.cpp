# Coverage build-type helpers
# Usage: cmake -DCMAKE_BUILD_TYPE=Coverage ..
#        make coverage

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(COVERAGE_FLAGS "--coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS_COVERAGE "${COVERAGE_FLAGS}" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage" CACHE STRING "" FORCE)
endif()

# Coverage report target — runs gcovr after the test suite
find_program(GCOVR gcovr)
if(GCOVR)
    add_custom_target(coverage
        COMMAND ${GCOVR}
            --root ${CMAKE_SOURCE_DIR}
            --object-directory ${CMAKE_BINARY_DIR}
            --exclude '${CMAKE_SOURCE_DIR}/tests/'
            --exclude '${CMAKE_SOURCE_DIR}/third_party/'
            --html-details ${CMAKE_BINARY_DIR}/coverage/coverage.html
            --json ${CMAKE_BINARY_DIR}/coverage/coverage.json
            --print-summary
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report with gcovr"
    )
    message(STATUS "Coverage: gcovr found, 'make coverage' target available")
else()
    message(STATUS "Coverage: gcovr not found, 'make coverage' target unavailable")
endif()
