/*
 * Copyright (c) 2022 Matt Young. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 */
#include "life.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/// Game of Life field. Stored as a 1D array, although it's actually 2D. True if cell is active,
/// false if it's dead.
static bool *grid = NULL;
/// Field copy, used for updating
static bool *nextGrid = NULL;
/// How many neighbours each cell in the grid has
static uint8_t *neighbourTable = NULL;
/// Field width and height in cells
static uint32_t gridWidth = 0, gridHeight = 0;

typedef struct {
    uint32_t x, y;
} Point_t;

/**
 * Sets a cell in Game of Life, accounting for out of bounds
 * @param gridPtr pointer to grid to update
 * @param x x coord of cell
 * @param y y coord of cell
 * @param value true if cell alive, false if cell dead
 */
static inline void setCell(bool *gridPtr, uint32_t x, uint32_t y, bool value) {
    if (x < 0 || y < 0 || x >= gridWidth || y >= gridHeight) {
        // out of bounds
        return;
    }
    gridPtr[x + gridWidth * y] = value;
}

/// Gets a cell from the GoL field, accounting for wrapping
static inline bool getCell(uint32_t x, uint32_t y) {
    if (x < 0 || y < 0 || x >= gridWidth || y >= gridHeight) {
        // out of bounds
        return 0;
    }
    return grid[x + gridWidth * y];
}

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

void lifeUpdate(void) {
    // 0. Clear old data
    memset(nextGrid, 0, gridWidth * gridHeight * sizeof(bool));
    // optimisation: may not need this, because we overwrite it anyway
    memset(neighbourTable, 0, gridWidth * gridHeight * sizeof(uint8_t));

    // 1. Calculate neighbours
    // optimisation: do step 1 and 2 in the same loop
    for (uint32_t y = 0; y < gridHeight; y++) {
        for (uint32_t x = 0; x < gridWidth; x++) {
            // do we need to check out of bounds?
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
}

void lifeInit(uint32_t width, uint32_t height) {
    grid = calloc(width * height, sizeof(bool));
    nextGrid = calloc(width * height, sizeof(bool));
    neighbourTable = calloc(width * height, sizeof(uint8_t));
    gridWidth = width;
    gridHeight = height;
}

void lifeInsertPatternPlainText(const char *filename, uint32_t x, uint32_t y) {
    FILE *f = fopen(filename, "r");
    while (true) {
        size_t lineSize = 0;
        char *line = NULL;
        if (getline(&line, &lineSize, f) == -1) {
            // got EOF, exit our reading loop
            free(line);
            break;
        }

        free(line);
    }
    fclose(f);
}

void lifeRenderConsole(void) {
    for (uint32_t y = 0; y < gridHeight; y++) {
        for (uint32_t x = 0; x < gridHeight; x++) {
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

void lifeDestroy(void) {
    free(grid);
    free(nextGrid);
    free(neighbourTable);
}