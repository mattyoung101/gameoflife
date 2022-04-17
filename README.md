# Conway's Game of Life
This project implements Conway's Game of Life in C. The idea is to start out with the most basic
approach possible, then progressively optimise it to see how fast I can get it to go.

Graphics are rendered using the GPU-accelerated APIs of SDL2, so you will need an OpenGL system 
(maybe DirectX would work too, who knows).

The debug build uses the Google Sanitizers to check for memory issues (which introduce considerable
slowdown). The release build runs as fast as possible, using `-O3 -march=native -mtune=native` to try
and get the most out of each platform. I'll also try and experiment with PGO in future.

In terms of compilers, I'm using  Clang 12 right now (but I'll benchmark different compilers later 
on down the track). It should compile under GCC as well. Also, I'm using CLion as my IDE.

## Features
- Full implementation of Game of Life
- Load patterns in both plain text (.txt) and run length encoded (.rle) format
- Pause and single-step mode
- Maximum allowable framerate control
- Performance logger
- GPU-accelerated rendering using SDL2

### Future features
- Zoom and pan
- Export the current grid to disk

## Results
### Setup
All proper tests will be executed on my Intel i5-7600k, NVIDIA GTX 1060 6GB, 16 GB RAM system for now.

Performance is measured by running the turing machine pattern with a grid size of 1715x1648,
i.e. the following command: `./gameoflife --pattern=../data/patterns/turingmachine.rle --grid=1715x1648`

### Implementation 1 (v0.1.0-noopt)
The most basic implementation. No optimisation outside of compiler options attempted. In saying this
though, I haven't written _intentionally_ slow code, I just haven't attempted any major optimisations
yet.

### Implementation 2 (v0.1.0-omp)
- Parallelized grid update and screen update loops using OpenMP
- Removed un-necessary memset to clear the neighbour table (the neighbour table gets overwritten
anyway)
- Inlined neighbour counting loop to cell update loop (now there's not two separate loops to calculate
neighbours and update the grid, which should improve threaded performance)
- Performed profile guided optimization **(note: not currently enabled, doesn't work too well yet)**

## Building and running
You will need:

- SDL 2.0.10 or newer (`sudo apt install libsdl2-2.0-0 libsdl2-dev libsdl2-2.0-0-dbgsym`)
- A POSIX compliant system that supports OpenGL

To understand how to use the program, try `./gameoflife --help`

## Licence
Mozilla Public Licence v2.0