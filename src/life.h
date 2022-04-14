// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>

/**
 * Initialises the Game of Life
 * @param width width of play field in cells
 * @param height height of play field in cells
 */
void lifeInit(uint32_t width, uint32_t height);

/// Frees memory associated with lifeInit()
void lifeDestroy(void);


/// Increments the world by one tick
void lifeUpdate(void);


/// Renders the current grid to the console.
void lifeRenderConsole(void);

/// Renders the current grid to an SDL renderer and texture.
void lifeRenderSDL(SDL_Texture *texture);

/**
 * Inserts a pattern, encoded in plain text format, into the grid. The (x,y) parameters are where the
 * pattern will be inserted into the grid, relative to the upper left hand cell of the pattern.
 * For example, an origin of (x,y) will insert the pattern at the top left of the grid.
 *
 * See documentation for this format here: https://conwaylife.com/wiki/Plaintext
 *
 * Errors: This function will cause an assertion failure if the current grid is too small to hold
 * the requested pattern, if the pattern file cannot be opened, if the coordinates are invalid,
 * and other such errors.
 * @param filename path to plain text grid file
 * @param oX where to insert the pattern on the current grid: x-coord
 * @param oY where to insert the pattern on the current grid: y-coord
 */
void lifeInsertPatternPlainText(const char *filename, uint32_t oX, uint32_t oY);

/**
 * Same as `lifeInsertPatternPlainText` but imports run length encoded (RLE) patterns.
 * See `lifeInsertPatternPlainText` for more info.
 * @param filename path to run length encoded grid file
 * @param x where to insert the pattern on the current grid: x-coord
 * @param y where to insert the pattern on the current grid: y-coord
 */
void lifeInsertPatternRLE(const char *filename, uint32_t x, uint32_t y);

/// Returns the number of generations that have passed.
uint64_t lifeGetGenerations(void);