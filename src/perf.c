// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#include "perf.h"
#include <string.h>

void perfUpdate(PerfCounter_t *counter, double time) {
    if (time < counter->min) {
        // new min value
        counter->min = time;
    } else if (time > counter->max) {
        // new max value
        counter->max = time;
    }
    counter->sum += time;
    counter->count++;
}

void perfClear(PerfCounter_t *counter) {
    memset(counter, 0, sizeof(PerfCounter_t));
}

void perfDumpConsole(PerfCounter_t *counter, const char *tag) {
    double avg = counter->sum / (double) counter->count;
    printf("[%s] min/max/avg: %.2f/%.2f/%.2f\n", tag, counter->min, counter->max, avg);
}