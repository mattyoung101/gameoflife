# Conway's Game of Life
This project implements Conway's Game of Life in C. The idea is to start out with the most basic
approach possible, then progressively optimise it to see how fast I can get it to go. Each version
will be stored on its own branch so it's possible to benchmark and compare all of them. The master 
branch will have the most up-to-date version.

Graphics are rendered using OpenGL 4.4. The window is created with GLFW, and OpenGL is loaded by
GLAD.

Memory safety testing is done using Google Sanitizers. I'm compiling with Clang 12 right now 
(but I'll benchmark different compilers later on down the track). Also using CLion as my IDE.

## Resullts
### Implementation 1 (this branch) 
The most basic implementation. Implements Game of Life rules. 
No optimisation outside of compiler options attempted.

## Building and running
You will need:

- GLFW (`sudo apt install libglfw3 libglfw3-dev`)
- OpenGL 4.4 or newer

## Licence
Mozilla Public Licence v2.0