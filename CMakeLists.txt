cmake_minimum_required(VERSION 3.31)
project(sis)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME}
        src/sis/BinaryReader.cpp
        src/sis/legacy/Parser.cpp
        src/sis/v9/Parser.cpp

        src/sis/c_api.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC ./include)

#
# ZLIB
#
find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_subdirectory(./examples/c_api)
