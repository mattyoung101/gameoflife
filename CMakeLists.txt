cmake_minimum_required(VERSION 3.21)
project(gameoflife C CXX)

set(CMAKE_C_STANDARD 11)

# Compile options and optimisation
add_compile_options(-Wall -Wextra -Wno-unused-parameter -g3)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Release build, enabling performance")
    add_compile_options(-O3 -march=native -mtune=native -flto)
    add_link_options(-flto)
else()
    message(STATUS "Debug build, enabling sanitizers")
    add_compile_options(-O0 -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

include_directories(lib/glad/include)
include_directories(lib/log)
include_directories(src)
add_executable(gameoflife src/main.c lib/glad/src/glad.c src/life.c src/defines.h src/perf.c
    src/perf.h src/utils.c src/utils.h lib/log/log.c lib/log/log.h)
target_link_libraries(gameoflife dl)

# SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(gameoflife ${SDL2_LIBRARIES})