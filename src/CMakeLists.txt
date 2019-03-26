# Include SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Local libs
add_library(Chip8_lib STATIC Chip8.cc)

# Create main exec
add_executable(chip8 main.cc)

# Link
target_link_libraries(chip8 ${SDL2_LIBRARIES} Chip8_lib)
