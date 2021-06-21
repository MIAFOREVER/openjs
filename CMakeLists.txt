cmake_minimum_required(VERSION 3.0.0)
project(openjs VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_BUILD_TYPE Debug)

add_executable(main
    src/token.cpp
    src/main.cpp
    src/log/log.cpp
)

target_include_directories(main
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
