#set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required (VERSION 3.4)
project(Chip8)
enable_testing()

add_subdirectory(extern/googletest/ build/)

add_subdirectory(src/)
add_subdirectory(tests/)
