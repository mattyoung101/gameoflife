/*
 * Copyright (c) 2022 Matt Young. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

static void errorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW error code %d: %s\n", error, description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(int argc, char *argv[]) {
    printf("Launching Game of Life...\n");

    // GLFW initialisation
    GLFWwindow *window;
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    // window creation
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    window = glfwCreateWindow(1280, 720, "Conway's Game of Life", NULL, NULL);
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
    glViewport(0, 0, 1280, 720);

    while (!glfwWindowShouldClose(window)) {
        // TODO time loops to get FPS

        // rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
