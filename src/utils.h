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

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 * Parses a size string in the format "[width]x[height]" with error checking
 * @param size size string (not modified)
 * @param widthOut pointer to store width component in
 * @param heightOut pointer to store height component in
 */
void utilsParseSize(const char *size, uint32_t *widthOut, uint32_t *heightOut);