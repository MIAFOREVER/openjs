cmake_minimum_required(VERSION 3.0.0)
project(openjs VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

ADD_DEFINITIONS(-I/usr/lib/llvm-6.0/include -fuse-ld=gold -Wl,--no-keep-files-mapped -Wl,--no-map-whole-files -fPIC -fvisibility-inlines-hidden -Werror=date-time -std=c++17 -Wall -W -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long -Wno-maybe-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -ffunction-sections -fdata-sections -O2 -DNDEBUG  -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

link_directories(/usr/lib/llvm-6.0/lib)

add_executable(main
    src/tokenizer.cpp
    src/main.cpp
    src/log/log.cpp
    src/token.cpp
    src/codegen.cpp
)

target_include_directories(main
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(main -lLLVM-6.0)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
