cmake_minimum_required(VERSION 3.20)

project(sis_c_api_example LANGUAGES C)

add_executable(${PROJECT_NAME}
        main.c
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        sis
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)