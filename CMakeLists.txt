cmake_minimum_required(VERSION 3.29)

project(decompress-uecrash-file)

add_subdirectory(zlib-1.3.1)
add_executable(duef duef.c)
add_definitions(-D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)

target_include_directories(duef PUBLIC zlib-1.3.1)

target_link_libraries(duef zlibstatic)