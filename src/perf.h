// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/// Performance timer data structure. All units should be in milliseconds.
typedef struct {
    /// Min value
    double min;
    /// Max value
    double max;

    /// Sum of all values, used for average
    double sum;
    /// Count of items recorded, used for average
    size_t count;
} PerfCounter_t;

/// Updates the performance counter with the given time in milliseconds
void perfUpdate(PerfCounter_t *counter, double time);

/// Clears the performance counter
void perfClear(PerfCounter_t *counter);

/// Dumps the values of the performance counter to the console
void perfDumpConsole(PerfCounter_t *counter, const char *tag);