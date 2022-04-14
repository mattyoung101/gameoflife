// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#include <stdio.h>
#include "glad/glad.h"
#include "life.h"
#include "defines.h"
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include "perf.h"
#include <SDL.h>
#include <assert.h>

static bool paused = false;
static PerfCounter_t perf = {0};
static struct option longOpts[] = {
        // Enable console rendering. Does not initialise the GUI.
        { "console-renderer", no_argument, NULL, 0 },
        // GoL grid size in cells, format is "[width]x[height]". Defaults to "64x64"
        { "grid-size", optional_argument, NULL, 0 },
        // Window size in pixels, format is "[width]x[height]". Defaults to "1600x900".
        { "window-size", optional_argument, NULL, 0 },
        { NULL, 0, NULL, 0}
};

static void printSDLVersion(void) {
    SDL_version sdlVersionLinked;
    SDL_GetVersion(&sdlVersionLinked);
    printf("[main] Using SDL v%d.%d.%d.\n", sdlVersionLinked.major, sdlVersionLinked.minor,
           sdlVersionLinked.patch);
}

int main(int argc, char *argv[]) {
    printf("[main] launching Game of Life...\n");
    printSDLVersion();

    // SDL setup
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        fprintf(stderr, "[main] Failed to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Window *window = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, DEFAULT_WINDOW_WIDTH,
                                          DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    assert(window != NULL);

    SDL_Renderer *render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    assert(render != NULL);
    SDL_Surface *bitmapSurface = SDL_LoadBMP("../data/testimage.bmp");
    assert(bitmapSurface != NULL);
    SDL_Texture *bitmapTex = SDL_CreateTextureFromSurface(render, bitmapSurface);
    SDL_FreeSurface(bitmapSurface);

    // initialise game of life
    lifeInit(DEFAULT_GRID_WIDTH, DEFAULT_GRID_HEIGHT);
    lifeInsertPatternPlainText("../data/patterns/4812diamond.txt", 0, 0);

    // main loop of graphical program
    bool shouldQuit = false;
    double printTimer = 0.0;
    double resetTimer = 0.0;
    while (!shouldQuit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldQuit = true;
            } else if (event.type == SDL_KEYUP) {
                if (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    shouldQuit = true;
                }
            }
        }
        double begin = SDL_GetTicks();

//        lifeUpdate();
//        lifeRenderConsole();
//        SDL_Delay(1000);

        SDL_RenderClear(render);
        SDL_RenderCopy(render, bitmapTex, NULL, NULL);
        SDL_RenderPresent(render);

        double delta = SDL_GetTicks() - begin;
        printTimer += delta;
        resetTimer += delta;
        if (printTimer >= 1000.0) {
            perfDumpConsole(&perf, "ms per frame");
            printTimer = 0.0;
        }
        if (resetTimer >= 2000.0) {
            perfClear(&perf);
            resetTimer = 0.0;
        }
        perfUpdate(&perf, delta * 1000.0);
    }

    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
    lifeDestroy();
    return 0;
}
