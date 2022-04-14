// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#include <stdio.h>
#include "life.h"
#include "defines.h"
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include "perf.h"
#include "log.h"
#include <SDL.h>
#include <assert.h>
#include "utils.h"

static bool paused = false;
static PerfCounter_t perf = {0};

// Command line options:
// GoL grid size in cells, format is "[width]x[height]". Defaults to "64x64"
// Window size in pixels, format is "[width]x[height]". Defaults to "1600x900".
// Maximum framerate. Defaults to -1, unlimited.
// Initial pattern to load.
// Whether to enable graphics or not (for performance testing).

static void printSDLVersion(void) {
    SDL_version sdlVersionLinked;
    SDL_GetVersion(&sdlVersionLinked);
    log_info("Using SDL v%d.%d.%d", sdlVersionLinked.major, sdlVersionLinked.minor,
           sdlVersionLinked.patch);
}

static double getTime() {
    return (double) SDL_GetPerformanceCounter() / (double) SDL_GetPerformanceFrequency();
}

int main(int argc, char *argv[]) {
    log_info("Conway's Game of Life v" VERSION);
    log_info("Copyright (c) 2022 Matt Young. Available under the Mozilla Public Licence 2.0.");
    printSDLVersion();
    log_set_level(LOG_DEBUG);

    int windowWidth = DEFAULT_WINDOW_WIDTH;
    int windowHeight = DEFAULT_WINDOW_HEIGHT;
    uint32_t gameWidth = DEFAULT_GRID_WIDTH;
    uint32_t gameHeight = DEFAULT_GRID_HEIGHT;

    // SDL setup
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        log_error("Failed to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    // TODO decide window size based on actual game of life field size
    SDL_Window *window = SDL_CreateWindow("Game of Life",
                                          SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                                          windowWidth, windowHeight,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    assert(window != NULL);

    SDL_Renderer *render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    assert(render != NULL);
    SDL_RendererInfo renderInfo;
    SDL_GetRendererInfo(render, &renderInfo);
    log_info("Using renderer: %s", renderInfo.name);

    SDL_Texture *gameTexture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGB888,
                                                 SDL_TEXTUREACCESS_STREAMING,
                                                 DEFAULT_GRID_WIDTH, DEFAULT_GRID_HEIGHT);
    assert(gameTexture != NULL);

    // initialise game of life
    lifeInit(gameWidth, gameHeight);
    lifeInsertPatternPlainText("../data/patterns/gosperglidergun.txt", 0, 0);

    // viewport for game of life
    // FIXME have this support resize
    SDL_Rect viewport = {0};
    // https://stackoverflow.com/a/1373879/5007892
    int scaleFactor = MIN(windowWidth / gameWidth, windowHeight / gameHeight);
    if (scaleFactor <= 0) scaleFactor = 1;
    viewport.w = ((int) gameWidth * scaleFactor) - 32;
    viewport.h = ((int) gameHeight * scaleFactor) - 32;
    // https://stackoverflow.com/a/27913142/5007892
    viewport.x = (windowWidth - viewport.w) / 2;
    viewport.y = (windowHeight - viewport.h) / 2;

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
                if (event.type == SDL_KEYUP && (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ||
                        event.key.keysym.scancode == SDL_SCANCODE_Q)) {
                    shouldQuit = true;
                }
            }
        }
        double begin = getTime();

        // update GoL
        // TODO add zoom
        lifeUpdate();

        // update graphics
        SDL_SetRenderDrawColor(render, 0x80, 0x80, 0x80, 0xFF);
        SDL_RenderClear(render);
        lifeRenderSDL(gameTexture);
        SDL_RenderCopy(render, gameTexture, NULL, &viewport);
        SDL_RenderPresent(render);

        // TODO add slow down parameter
        //SDL_Delay(15);

        // update performance counters
        double end = getTime();
        double delta = (end - begin) * 1000.0;
        printTimer += delta;
        resetTimer += delta;
        if (printTimer >= 1000.0) {
            perfDumpConsole(&perf, "FPS");
            printTimer = 0.0;
        }
        if (resetTimer >= 2000.0) {
            perfClear(&perf);
            resetTimer = 0.0;
        }
        perfUpdate(&perf, 1000.0 / delta);
    }

    SDL_DestroyTexture(gameTexture);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
    lifeDestroy();
    return 0;
}
