cmake_minimum_required(VERSION 3.21)
project(gameoflife C CXX)

set(CMAKE_C_STANDARD 11)

# Compile options and optimisation
add_compile_options(-Wall -Wextra -Wno-unused-parameter -g3)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Release build, enabling performance")
    add_compile_options(-O3 -march=native -mtune=native -flto)
    add_link_options(-flto)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "ReleasePGO")
    message(STATUS "Release build with profile guided optimisation")
    # Use LLVM IR PGO (or its alternative in GCC):
    # - https://source.android.com/devices/tech/perf/pgo
    # - https://clang.llvm.org/docs/UsersManual.html#profiling-with-instrumentation
    add_compile_options(-O3 -march=native -mtune=native -flto -fprofile-instr-generate=prof/gol-%p.profraw)
    add_link_options(-flto -fprofile-instr-generate=prof/gol-%p.profraw)
else()
    message(STATUS "Debug build, enabling sanitizers")
    add_compile_options(-O0 -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

include_directories(lib/glad/include)
include_directories(lib/log)
include_directories(lib/argtable3)
include_directories(src)
add_executable(gameoflife src/main.c lib/glad/src/glad.c src/life.c src/defines.h src/perf.c
    src/perf.h src/utils.c src/utils.h lib/log/log.c lib/log/log.h lib/argtable3/argtable3.c
    lib/argtable3/argtable3.h)
target_link_libraries(gameoflife dl)

# SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(gameoflife ${SDL2_LIBRARIES})

# OpenMP
find_package(OpenMP REQUIRED)
target_link_libraries(gameoflife OpenMP::OpenMP_C OpenMP::OpenMP_CXX)