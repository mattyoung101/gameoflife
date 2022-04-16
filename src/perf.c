// Copyright (c) 2022 Matt Young. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at
// http://mozilla.org/MPL/2.0/.
#include "perf.h"
#include "log.h"
#include <string.h>

void perfUpdate(PerfCounter_t *counter, double time) {
    if (time <= counter->min) {
        // new min value
        counter->min = time;
    } else if (time >= counter->max) {
        // new max value
        counter->max = time;
    }
    counter->sum += time;
    counter->count++;
}

void perfClear(PerfCounter_t *counter) {
    memset(counter, 0, sizeof(PerfCounter_t));
    // initialise min to a large positive number, and max to a large negative number, such that the
    // first time we run we will always set those values
    counter->min = 999999999;
    counter->max = -99999999;
}

void perfDumpConsole(PerfCounter_t *counter, const char *tag) {
    double avg = counter->sum / (double) counter->count;
    log_debug("[%s] min/max/avg: %.2f/%.2f/%.2f", tag, counter->min, counter->max, avg);
}