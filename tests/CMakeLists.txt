# --- Test Configuration ---
# Define a list of tests to build.
# The convention is that for a test named 'foo', the source file is 'test_foo.cpp'
# and the executable will be 'test_foo'.
set(TEST_LIST
    math
    ecs
)

set(BUILT_TESTS "")
foreach(TEST_NAME ${TEST_LIST})
    set(TEST_SOURCE "test_${TEST_NAME}.cpp")
    set(TEST_EXECUTABLE "test_${TEST_NAME}")

    add_executable(${TEST_EXECUTABLE} ${TEST_SOURCE})
    # FactoryGameLib provides all necessary include directories and links via PUBLIC inheritance
    target_link_libraries(${TEST_EXECUTABLE} PRIVATE FactoryGameLib)

    add_test(NAME "${TEST_NAME}Tests" COMMAND ${TEST_EXECUTABLE})
    list(APPEND BUILT_TESTS "${TEST_NAME}Tests")
endforeach()

# Set the working directory for all tests defined in this file
if(BUILT_TESTS)
    set_tests_properties(${BUILT_TESTS}
        PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
