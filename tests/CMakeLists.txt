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

include(GoogleTest)

gtest_discover_tests(sis_tests)
