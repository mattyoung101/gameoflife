// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#include "utils.h"

void utilsParseSize(const char *size, uint32_t *widthOut, uint32_t *heightOut) {
    char *copy = strdup(size);
    if (strchr(copy, 'x') == NULL) {
        // no x character found, so strtok would fail
        goto die;
    }
    char *widthStr = strtok(copy, "x");
    char *heightStr = strtok(NULL, "x");

    char *endptr = NULL;
    *widthOut = strtol(widthStr, &endptr, 10);
    if (strlen(endptr) > 0) {
        goto die;
    }

    endptr = NULL;
    *heightOut = strtol(heightStr, &endptr, 10);
    if (strlen(endptr) > 0) {
        goto die;
    }

    // return here, so we don't jump into our goto accidentally (also free-ing copy to avoid memory leak)
    free(copy);
    return;

    die:
    fprintf(stderr, "Invalid size string %s", size);
    free(copy);
    exit(1);
}

bool utilsStartsWith(const char *prefix, const char *str) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}