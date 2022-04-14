# Conway's Game of Life
This project implements Conway's Game of Life in C. The idea is to start out with the most basic
approach possible, then progressively optimise it to see how fast I can get it to go. Each version
will be stored on its own branch so it's possible to benchmark and compare all of them. The master 
branch will have the most up-to-date version.

Graphics are currently using SDL. Previously I had a go at using OpenGL via glfw+GLAD, but I decided
it was easier to just blit things to the screen using SDL or Allegro, especially since this isn't
really a graphics project.

Memory safety testing is done using Google Sanitizers. I'm compiling with Clang 12 right now 
(but I'll benchmark different compilers later on down the track). Also using CLion as my IDE.

## Results
### Implementation 1 (this branch) 
The most basic implementation. No optimisation outside of compiler options attempted. In saying this
though, I haven't written _intentionally_ slow code, and although I've identified places I can
optimise the code already, I've only written a comment and haven't written that up yet.

## Building and running
You will need:

- SDL 2.0.10 or newer (`sudo apt install libsdl2-2.0-0 libsdl2-dev`)
- A POSIX compliant system that supports OpenGL

## Licence
Mozilla Public Licence v2.0