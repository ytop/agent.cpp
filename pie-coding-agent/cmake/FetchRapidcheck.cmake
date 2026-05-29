include(FetchContent)

FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG        b2d9ed2dddefc4b84318d664b4f221eb792d89c7
    GIT_SHALLOW    FALSE
)

# Disable rapidcheck's own test suite
set(RC_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(RC_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RC_ENABLE_CATCH ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(rapidcheck)
