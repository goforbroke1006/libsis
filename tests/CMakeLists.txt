include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.18.x
)

FetchContent_MakeAvailable(googletest)

add_executable(sis_tests
        sis/BinaryReader.cpp
)

target_link_libraries(sis_tests
        PRIVATE
        sis
        GTest::gtest_main
)

# Copy all DDLs to the tests directory for Win32 build.
if (WIN32)
    add_custom_command(
            TARGET sis_tests
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:sis_tests>
            $<TARGET_FILE_DIR:sis_tests>
            COMMAND_EXPAND_LISTS
    )
    #add_custom_command(
    #        TARGET sis_tests
    #        POST_BUILD
    #        COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #        $<TARGET_FILE:sis>
    #        $<TARGET_FILE_DIR:sis_tests>
    #)
endif ()

include(GoogleTest)

gtest_discover_tests(sis_tests)
