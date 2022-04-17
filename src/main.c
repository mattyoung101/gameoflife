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
#include "argtable3.h"

static PerfCounter_t perf = {0};

// Command line options:
// GoL grid size in cells, format is "[width]x[height]". Defaults to "64x64"
// Window size in pixels, format is "[width]x[height]". Defaults to "1600x900".
// Maximum framerate. Defaults to -1, unlimited.
// Initial pattern to load.
// Coordinates on where to insert initial pattern.
// Whether to enable graphics or not (for performance testing).

/// Print runtime SDL version
static void printSDLVersion(void) {
    SDL_version sdlVersionLinked;
    SDL_GetVersion(&sdlVersionLinked);
    log_info("Using SDL v%d.%d.%d", sdlVersionLinked.major, sdlVersionLinked.minor,
           sdlVersionLinked.patch);
}

/// Get a high resolution time in SECONDS using SDL_GetPerformanceCounter
static double getTime(void) {
    return (double) SDL_GetPerformanceCounter() / (double) SDL_GetPerformanceFrequency();
}

/// Updates the window title for when the game is paused
static void updatePausedWindowTitle(SDL_Window *window) {
    char buf[256] = {0};
    snprintf(buf, 256, "Game of Life (paused on generation %lu)",
             lifeGetGenerations());
    SDL_SetWindowTitle(window, buf);
}

static SDL_Rect calculateViewport(int windowWidth, int windowHeight,
                                  uint32_t gameWidth, uint32_t gameHeight) {
    SDL_Rect viewport = {0};
    // https://stackoverflow.com/a/1373879/5007892
    double scaleFactor = fmin((double) windowWidth / gameWidth, (double) windowHeight / gameHeight);
    if (scaleFactor <= 0) {
        // don't allow sub-zero sizes
        scaleFactor = 1;
    }
    viewport.w = ((int) round(gameWidth * scaleFactor)) - 32;
    viewport.h = ((int) round(gameHeight * scaleFactor)) - 32;
    // https://stackoverflow.com/a/27913142/5007892
    viewport.x = (windowWidth - viewport.w) / 2;
    viewport.y = (windowHeight - viewport.h) / 2;

    log_debug("Scale factor: %.2f", scaleFactor);
    log_trace("Game viewport (x,y,w,h): %d,%d,%d,%d", viewport.x, viewport.y, viewport.w, viewport.h);
    return viewport;
}

int main(int argc, char *argv[]) {
    // parse program arguments using argtable3
    // TODO add argument for coordinates on where to insert pattern
    // TODO add argument for maximum frames per second
    struct arg_lit *argHelp = arg_lit0(NULL, "help", "Display help and exit.");
    struct arg_str *argGrid = arg_str0(NULL, "grid", "[width]x[height]",
               "Game of Life grid size in cells.  Defaults to 256x256.");
    struct arg_str *argWin = arg_str0(NULL, "window","[width]x[height]",
              "Window size. Format is \"[width]x[height]\". Defaults to 1600x900");
    struct arg_lit *argGraphics = arg_lit0(NULL, "no-graphics",
           "Disable graphical rendering, for performance testing.");
    struct arg_int *argFps = arg_int0(NULL, "max-fps", "int",
            "Maximum framerate, or -1 to unlock. Defaults to unlocked");

    struct arg_file *argPattern = arg_file1(NULL, "pattern", "file",
            "Pattern file, use .rle for RLE encoded files and .txt for plaintext files.");

    struct arg_end *argEnd = arg_end(20);

    void *argtable[] = {argHelp, argGrid, argWin, argGraphics, argFps,
                        argPattern, argEnd};
    assert(arg_nullcheck(argtable) == 0);

    // Set defaults for arg parser
    *argGrid->sval = (XSTR(DEFAULT_GRID_WIDTH) "x" XSTR(DEFAULT_GRID_HEIGHT));
    *argWin->sval = (XSTR(DEFAULT_WINDOW_WIDTH) "x" XSTR(DEFAULT_WINDOW_HEIGHT));
    *argFps->ival = -1;

    int nerrors = arg_parse(argc, argv, argtable);
    if (argHelp->count > 0) {
        // special case: help command, print syntax and exit
        printf("Conway's Game of Life v" VERSION "\n");
        printf("Copyright (c) 2022 Matt Young. Available under the Mozilla Public Licence 2.0.\n");
        printf("Keyboard controls:\n- SPACE to toggle pause\n- RIGHT ARROW to single step "
               "while paused\n- Q or ESCAPE to quit\n\n");
        printf("Usage: gameoflife");
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-30s %s\n");
        arg_free(argtable);
        exit(0);
    } else if (nerrors > 0) {
        // arg parser failed, print errors and quit
        arg_print_errors(stderr, argEnd, "gameoflife");
        arg_free(argtable);
        exit(1);
    }

    log_info("Conway's Game of Life v" VERSION);
    log_info("Copyright (c) 2022 Matt Young. Available under the Mozilla Public Licence 2.0.");
    printSDLVersion();
    log_set_level(LOG_DEBUG);

    int windowWidth = 0, windowHeight = 0;
    uint32_t gameWidth = 0, gameHeight = 0;

    // Store arguments after parsing
    utilsParseSize(*argGrid->sval, &gameWidth, &gameHeight);
    utilsParseSize(*argWin->sval, (uint32_t*) &windowWidth, (uint32_t*) &windowHeight);
    const char *patternFile = *argPattern->filename;
    bool isPatternRLE = strcasecmp(*argPattern->extension, ".rle") == 0;
    bool graphicsDisabled = argGraphics->count > 0;
    int maxFramerate = *argFps->ival;
    if (maxFramerate <= 0 && maxFramerate != -1) {
        log_error("Max framerate must be either -1 to unlock, or a positive integer.");
        exit(1);
    }

    // SDL setup
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        log_error("Failed to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    // TODO decide window size based on actual game of life field size
    SDL_Window *window = SDL_CreateWindow("Game of Life (running)",
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
                                                 (int) gameWidth, (int) gameHeight);
    assert(gameTexture != NULL);

    // initialise game of life
    lifeInit(gameWidth, gameHeight);
    if (isPatternRLE) {
        lifeInsertPatternRLE(patternFile, 0, 0);
    } else {
        lifeInsertPatternPlainText(patternFile, 0, 0);
    }
    perfClear(&perf);

    // viewport for game of life
    SDL_Rect viewport = calculateViewport(windowWidth, windowHeight, gameWidth, gameHeight);

    // free argument parser
    arg_free(argtable);

    // main loop of graphical program
    bool shouldQuit = false;
    bool paused = false;
    bool advanceOneFrame = false;
    double printTimer = 0.0;
    double resetTimer = 0.0;

    while (!shouldQuit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldQuit = true;
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ||
                        event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    // press "Q" or escape or close window to quit
                    shouldQuit = true;
                } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    // press "space" to toggle pause
                    paused = !paused;
                    if (paused) {
                        updatePausedWindowTitle(window);
                    } else {
                        SDL_SetWindowTitle(window, "Game of Life (running)");
                        // reset performance counter after pausing
                        perfClear(&perf);
                        printTimer = 0.0;
                    }
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT && paused) {
                    // press right arrow to advance one frame (only when paused)
                    advanceOneFrame = true;
                }
            } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
                viewport = calculateViewport(windowWidth, windowHeight, gameWidth, gameHeight);
            }
        }
        double begin = getTime();

        // update GoL
        // TODO add zoom
        if (!paused) {
            // if not paused, always update
            lifeUpdate();
        } else if (advanceOneFrame) {
            // otherwise, if we are paused, we might need to advance one frame
            lifeUpdate();
            updatePausedWindowTitle(window);
            advanceOneFrame = false;
        }

        // update graphics
        SDL_SetRenderDrawColor(render, 0x80, 0x80, 0x80, 0xFF);
        SDL_RenderClear(render);
        lifeRenderSDL(gameTexture);
        SDL_RenderCopy(render, gameTexture, NULL, &viewport);
        SDL_RenderPresent(render);

        if (paused) {
            // in paused mode just run at 30 fps to save compute; and don't update performance
            // counters
            SDL_Delay(33);
            continue;
        }

        // update performance counters
        double end = getTime();
        double delta = (end - begin) * 1000.0;

        // FPS limiter (if it's enabled): if we spend less than the number of milliseconds
        // maxFramerate is (e.g. 30fps = 33.3ms), then we should delay for the remaining amount of
        // milliseconds to get the required framerate
        if (maxFramerate > 0 && delta < 1000.0 / maxFramerate) {
            SDL_Delay((int) round((1000.0 / maxFramerate) - delta));
            // re-calculate begin/end times after delay
            end = getTime();
            delta = (end - begin) * 1000.0;
        }

        printTimer += delta;
        resetTimer += delta;
        if (printTimer >= 1000.0) {
            perfDumpConsole(&perf, "FPS");
            printTimer = 0.0;
        }
        if (resetTimer >= 10000.0) {
            perfClear(&perf);
            resetTimer = 0.0;
        }
        perfUpdate(&perf, 1000.0 / delta);
    }

    lifeDestroy();
    SDL_DestroyTexture(gameTexture);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_VideoQuit();
    SDL_Quit();
    return 0;
}
