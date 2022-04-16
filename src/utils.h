// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/**
 * Parses a size string in the format "[width]x[height]" with error checking
 * @param size size string (not modified)
 * @param widthOut pointer to store width component in
 * @param heightOut pointer to store height component in
 */
void utilsParseSize(const char *size, uint32_t *widthOut, uint32_t *heightOut);

/**
 * Determines if a line starts with the given substring.
 *
 * Ref: https://stackoverflow.com/a/4770992/5007892
 * @param prefix string to determine if the line starts with
 * @param str the line
 * @return true if it starts with this, else false
 */
bool utilsStartsWith(const char *prefix, const char *str);