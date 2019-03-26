add_executable(test_suite main_test.cc)

#add_library(Chip8_lib STATIC ../src/Chip8.cc)

target_link_libraries(test_suite gtest Chip8_lib)

add_test(UnitTests test_suite)
