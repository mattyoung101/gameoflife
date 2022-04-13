/*
 * Copyright (c) 2022 Matt Young. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include "glad/glad.h"
#include "life.h"
#include "defines.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include "perf.h"

static bool paused = false;
static PerfCounter_t perf = {0};
static struct option longOpts[] = {
        // Enable console rendering. Does not initialise the GUI.
        { "console-renderer", no_argument, NULL, 0 },
        // GoL grid size in cells, format is "[width]x[height]". Defaults to "64x64"
        { "grid-size", optional_argument, NULL, 0 },
        // Window size in pixels, format is "[width]x[height]". Defaults to "1600x900".
        { "window-size", optional_argument, NULL, 0 },
        { NULL, 0, NULL, 0}
};

/**
 * Parses a size string in the format "[width]x[height]" with error checking
 * @param size size string (not modified)
 * @param widthOut pointer to store width component in
 * @param heightOut pointer to store height component in
 */
static void parseSize(const char *size, uint32_t *widthOut, uint32_t *heightOut) {
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

static void errorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW error code %d: %s\n", error, description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        paused = !paused;
        printf("Paused: %s\n", paused ? "true" : "false");
    }
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(int argc, char *argv[]) {
    printf("Launching Game of Life...\n");
    perf.resetTime = 5000;

    // GLFW initialisation
    GLFWwindow *window;
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    // window creation
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Conway's Game of Life", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(window, keyCallback);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialise GLAD\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // initialise game of life
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    lifeInit(64, 64);

    // allocate texture for drawing game of life to
    // https://learnopengl.com/Getting-started/Textures
    unsigned int texture;
    glGenTextures(1, &texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // TODO then we can do glTexImage2D

    double counter = 0.0;
    while (!glfwWindowShouldClose(window)) {
        double begin = glfwGetTime();

        // rendering
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (!paused) {
            lifeUpdate();
        }
        // TODO render Game of Life to texture, to screen

        glfwSwapBuffers(window);
        double delta = glfwGetTime() - begin;

        // update performance counter
        counter += delta;
        if (counter >= 1.0) {
            perfDumpConsole(&perf, "ms per frame");
            counter = 0.0;
        }
        perfUpdate(&perf, delta * 1000.0);

        glfwPollEvents();
    }

    lifeDestroy();
    glfwTerminate();
    return 0;
}
