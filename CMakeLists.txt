cmake_minimum_required(VERSION 3.31)
project(
        sis
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED
        src/sis/BinaryReader.cpp
        src/sis/legacy/Parser.cpp
        src/sis/v9/Parser.cpp

        src/sis/c_api.cpp
)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(sis PRIVATE
        $<$<CONFIG:Debug>:-O0 -g3 -fno-omit-frame-pointer>
)
set_target_properties(sis PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Version for .so
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 0
)

#
# ZLIB
#
find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ZLIB::ZLIB)

include(CTest)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()

add_subdirectory(./examples/c_api)

#
# Install
#

include(GNUInstallDirs)

install(
        TARGETS sis
        EXPORT sisTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
        EXPORT sisTargets
        FILE sisTargets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sis
)

#
# Generate sisConfig.cmake
#

include(CMakePackageConfigHelpers)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sisConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/sisConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sis
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/sisConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/sisConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/sisConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sis
)

#
# Uninstall
#

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        @ONLY
)

add_custom_target(uninstall
        COMMAND "${CMAKE_COMMAND}" -P
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)
