// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#include "life.h"
#include "log.h"
#include "utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

/// Game of Life field. Stored as a 1D array, although it's actually 2D. True if cell is active,
/// false if it's dead.
static bool *grid = NULL;
/// Field copy, used for updating
static bool *nextGrid = NULL;
/// How many neighbours each cell in the grid has
static uint8_t *neighbourTable = NULL;
/// Pixel data for SDL
static uint32_t *pixelData = NULL;
/// Field width and height in cells
static uint32_t gridWidth = 0, gridHeight = 0;
/// Current generation we are on
static uint64_t generations = 0;

typedef struct {
    uint32_t x, y;
} Point_t;

typedef enum {
    /// Accept a number
    PARSE_NUM = 0,
    /// Accept a tag: b, o, $
    PARSE_TAG,
} RLEParseState_t;

#define NUM_DIRECTIONS 8
static const Point_t directions[NUM_DIRECTIONS] = {{-1, -1},
                                                   {-1, 0},
                                                   {-1, 1},
                                                   {0,  -1},
                                                   {0,  1},
                                                   {1,  -1},
                                                   {1,  0},
                                                   {1,  1}
};

/**
 * Sets a cell in Game of Life, accounting for out of bounds
 * @param gridPtr pointer to grid to update
 * @param x x coord of cell
 * @param y y coord of cell
 * @param value true if cell alive, false if cell dead
 * @return true if the cell could be set successfully, else false
 */
static inline bool setCell(bool *gridPtr, uint32_t x, uint32_t y, bool value) {
    if (x < 0 || y < 0 || x >= gridWidth || y >= gridHeight) {
        // out of bounds
        return false;
    }
    gridPtr[x + gridWidth * y] = value;
    return true;
}

/// Gets a cell from the GoL field, accounting for wrapping
static inline bool getCell(uint32_t x, uint32_t y) {
    if (x < 0 || y < 0 || x >= gridWidth || y >= gridHeight) {
        // out of bounds
        return 0;
    }
    return grid[x + gridWidth * y];
}

/**
 * Insert multiple cells into the grid.
 * @param gridPtr pointer to the grid to update
 * @param x pointer to the current x position in the grid
 * @param y current y position in the grid
 * @param count how many cells to insert
 * @param value true if cell
 */
static void setCellMultiple(bool *gridPtr, uint32_t *x, uint32_t y, uint32_t count, bool value) {
    log_trace("Emitting %u %s cells starting at %u,%u", count, value ? "alive" : "dead", *x, y);
    for (uint32_t i = 0; i < count; i++) {
        if (!setCell(gridPtr, (*x)++, y, value)) {
            log_error("Failed to insert cell for RLE at %u,%u", x - 1, y);
            log_error("Please check the current grid size of %ux%u is large enough to hold the "
                      "pattern.", gridWidth, gridHeight);
            exit(1);
        }
    }
}

/// Calculates the sum of the neighbours of a cell in the GoL field
static uint8_t sumNeighbours(uint32_t x, uint32_t y) {
    uint8_t count = 0;
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        uint32_t dx = directions[i].x;
        uint32_t dy = directions[i].y;
        if (getCell(x + dx, y + dy)) {
            count++;
        }
    }
    return count;
}

void lifeInit(uint32_t width, uint32_t height) {
    if (width < 0 || height < 0) {
        log_error("Invalid grid size %dx%d", width, height);
        exit(1);
    }
    grid = calloc(width * height, sizeof(bool));
    nextGrid = calloc(width * height, sizeof(bool));
    neighbourTable = calloc(width * height, sizeof(uint8_t));
    pixelData = calloc(width * height, sizeof(uint32_t));
    gridWidth = width;
    gridHeight = height;
    log_info("Initialised %ux%u grid", width, height);
}

void lifeUpdate(void) {
    // 0. Clear old data
    memset(nextGrid, 0, gridWidth * gridHeight * sizeof(bool));
    // optimisation: may not need this, because we overwrite it anyway
    memset(neighbourTable, 0, gridWidth * gridHeight * sizeof(uint8_t));

    // 1. Calculate neighbours
    // optimisation: do step 1 and 2 in the same loop
    for (uint32_t y = 0; y < gridHeight; y++) {
        for (uint32_t x = 0; x < gridWidth; x++) {
            neighbourTable[x + gridWidth * y] = sumNeighbours(x, y);
        }
    }

    // 2. Apply Game of Life rules
    // Using Wikipedia condensed rules: https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life#Rules
    // Also see the Algorithms section in the above article (for why we need two grids)
    for (uint32_t y = 0; y < gridHeight; y++) {
        for (uint32_t x = 0; x < gridWidth; x++) {
            uint8_t neighbours = neighbourTable[x + gridWidth * y];
            // optimisation: use grid access directly, skip out of bounds check, since it shouldn't
            // be out of bounds
            bool alive = getCell(x, y);

            // TODO check these rule are correct
            // shorter condition: neighbours == 3 || (neighbours == 2 && alive)
            if (alive && (neighbours == 2 || neighbours == 3)) {
                // 1. Any live cell with two or three live neighbours survives
                // optimisation: do we really need this function call?
                setCell(nextGrid, x, y, true);
            } else if (!alive && neighbours == 3) {
                // 2. Any dead cell with three live neighbours becomes a live cell.
                setCell(nextGrid, x, y, true);
            } else {
                // 3. All other live cells die in the next generation. Similarly, all other dead cells stay dead.
                setCell(nextGrid, x, y, false);
            }
        }
    }

    // 3. Update grid
    memcpy(grid, nextGrid, gridWidth * gridHeight * sizeof(bool));
    generations++;
}

void lifeInsertPatternPlainText(const char *filename, uint32_t oX, uint32_t oY) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        log_error("Failed to open file %s for reading: %s", filename, strerror(errno));
        exit(1);
    }
    log_info("Reading plain text pattern %s", filename);
    uint32_t y = 0;
    size_t unused = 0;

    while (true) {
        char *line = NULL;
        // go over each line in the file
        // note we just assume the output of getline is null terminated (which it should be?) so
        // we ignore the "n" parameter
        if (getline(&line, &unused, f) == -1) {
            // got EOF, exit our reading loop
            free(line);
            break;
        }
        // skip comments
        if (line[0] == '!') {
            free(line);
            continue;
        }
        // iterate over each char in the line to get our x coordinates and set cells
        // note that in the plain text format, the "O" character means a cell is alive
        for (uint32_t x = 0; x < strlen(line); x++) {
            // if we failed to set the cell, raise an error
            if (!setCell(grid, oX + x, oY + y, line[x] == 'O')) {
                log_error("Failed to set cell at %u,%u for line: %s",oX + x, oY + y, line);
                log_error("Please check the current grid size of %ux%u can hold the pattern.",
                          gridWidth, gridHeight);
                exit(1);
            }
        }

        y++;
        free(line);
    }
    fclose(f);
}

void lifeInsertPatternRLE(const char *filename, uint32_t oX, uint32_t oY) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        log_error("Failed to open file %s for reading: %s", filename, strerror(errno));
        exit(1);
    }
    log_info("Reading RLE pattern %s", filename);
    uint32_t y = 0;
    uint32_t x = 0;
    size_t unused = 0;

    // find out when the preamble stops and the actual content of the file starts
    long contentIdx;
    while (true) {
        char *line = NULL;
        // getline seems to advance the file pointer to the next line, so we need to record where
        // we are before we get there (to tell the reader to start at the line after the preamble)
        long posBeforeRead = ftell(f);
        if (getline(&line, &unused, f) == -1) {
            // no content yet, file is just header data somehow
            log_error("Unexpected EOF while skipping RLE header");
            free(line);
            exit(1);
        }

        if (line[0] == '#' || utilsStartsWith("x = ", line)) {
            // starts with a hash or the prefix line, ignore
            free(line);
            continue;
        } else {
            // found first content line, mark our position and rewind
            contentIdx = posBeforeRead;
            log_trace("Reached RLE content at idx %zu", contentIdx);
            fseek(f, 0, SEEK_SET);
            free(line);
            break;
        }
    }

    // jump to content, we're now reading one char at a time
    fseek(f, contentIdx, SEEK_SET);

    char numBuf[32] = {0};
    int numBufIdx = 0;
    uint32_t tagCount = 1; // if no number is specified, we have one by default
    RLEParseState_t state = PARSE_NUM;
    while (true) {
        char c = (char) fgetc(f);
        if (feof(f) || c == '!') {
            // finish reading on EOF or !
            break;
        } else if (c == '\n' || c == '\r' || c == '\t' || c == ' ') {
            // skip whitespace
            continue;
        }

        if (state == PARSE_NUM) {
            if (isdigit(c)) {
                // continue reading number
                numBuf[numBufIdx++] = c;
                continue;
            } else {
                // end of number, parse the number and say that we're now reading a tag
                char *endptr = NULL;
                tagCount = strtol(numBuf, &endptr, 10);
                if (strlen(endptr) > 0) {
                    log_error("Unexpected non-digit character(s) parsing RLE: %s", endptr);
                    exit(1);
                } else if (tagCount == 0) {
                    // if we did not parse a digit, set it to 1 by default
                    log_trace("Setting 0 tag count to default 1");
                    tagCount = 1;
                }
                // seek back one char to re-read the tag type
                fseek(f, -1, SEEK_CUR);
                state = PARSE_TAG;
                log_trace("Read number %u", tagCount);
                continue;
            }
        } else /* state == PARSE_TAG */ {
            if (c == 'b') {
                // insert dead cells
                setCellMultiple(grid, &x, y, tagCount, false);
            } else if (c == 'o') {
                // insert alive cells
                setCellMultiple(grid, &x, y, tagCount, true);
            } else if (c == '$') {
                // go to next line(s)
                y += tagCount;
                x = 0;
                log_trace("End of line. Position is now %u,%u", y, x);
            } else {
                log_error("Illegal RLE tag: %c", c);
                exit(1);
            }
            // clear previous number and go and get the next number
            memset(numBuf, 0, 32 * sizeof(char));
            numBufIdx = 0;
            tagCount = 1;
            state = PARSE_NUM;
            continue;
        }
    }
    fclose(f);
}

void lifeRenderConsole(void) {
    for (uint32_t y = 0; y < gridHeight; y++) {
        for (uint32_t x = 0; x < gridWidth; x++) {
            bool alive = getCell(x, y);
            if (alive) {
                printf("O");
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

void lifeRenderSDL(SDL_Texture *texture) {
    // copy over grid data
    for (uint32_t y = 0; y < gridHeight; y++) {
        for (uint32_t x = 0; x < gridWidth; x++) {
            bool alive = getCell(x, y); // optimisation: don't use bounds checked version of this
            pixelData[x + gridWidth * y] = alive ? 0xFFFFFF : 0;
        }
    }
    SDL_UpdateTexture(texture, NULL, pixelData, gridWidth * sizeof(uint32_t));
}

void lifeDestroy(void) {
    free(pixelData);
    free(grid);
    free(nextGrid);
    free(neighbourTable);
}

uint64_t lifeGetGenerations(void) {
    return generations;
}