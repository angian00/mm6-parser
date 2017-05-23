#include <GL/glew.h>
#include <SDL.h>

#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <limits.h>
#include "geometry.h"


//----------------------------------------------------------------------
//--- global variables

GLuint program;
GLint attribute_coord2d;
GLuint vbo_lines;

//----------------------------------------------------------------------
//--- function declaration

int init_graphical_resources();
void free_graphical_resources();
void mainLoop(SDL_Window* window, uint32_t n_lines);
void render(SDL_Window* window, uint32_t n_lines);
//void normalize_outline_data();

//----------------------------------------------------------------------
//--- Implementation



int visualize2d(uint32_t n_lines, struct point *line_data) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("visualize2d",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (window == NULL) {
        fprintf(stderr, "!! Error: can't create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
    if (SDL_GL_CreateContext(window) == NULL) {
        fprintf(stderr, "!! SDL_GL_CreateContext: %s \n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GL_CreateContext(window);

    printf("--Created SDL context \n");

    //glEnable (GL_LINE_SMOOTH);
    //glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth (1.5);

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        fprintf(stderr, "!! Error: glewInit: %s \n", glewGetErrorString(glew_status));
        return EXIT_FAILURE;
    }

    if (!GLEW_VERSION_2_0) {
        fprintf(stderr, "!! Error: your graphic card does not support OpenGL 2.0 \n");
        return EXIT_FAILURE;
    }

    printf("--initialized GLEW \n");


    //read_outline_data_fake();

//    normalize_outline_data();
    printf("--read outline coordinates \n");


    if (init_graphical_resources(n_lines, line_data) == EXIT_FAILURE)
        return EXIT_FAILURE;

    printf("--initialized graphical resources \n");

    mainLoop(window, n_lines);

    free_graphical_resources();

    return EXIT_SUCCESS;
}


void mainLoop(SDL_Window* window, uint32_t n_lines) {
    while (1) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                return;
        }

        render(window, n_lines);
    }
}


int init_graphical_resources(uint32_t n_lines, struct point *line_data) {
    GLint compile_ok = GL_FALSE;
    GLint link_ok = GL_FALSE;

    //----- vertex shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const char *vs_source =
        //"#version 100\n"  // OpenGL ES 2.0
        "#version 120\n"  // OpenGL 2.1
        "attribute vec2 coord2d;                  "
        "void main(void) {                        "
        "  gl_Position = vec4(coord2d, 0.0, 1.0); "
        "   gl_PointSize = 10.0;                  "
        "}";
    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);

    if (!compile_ok) {
        fprintf(stderr, "!! Error in vertex shader \n");
        return EXIT_FAILURE;
    }

    //----- fragment shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fs_source =
        //"#version 100\n"  // OpenGL ES 2.0
        "#version 120\n"  // OpenGL 2.1
        "void main(void) {        "
        "  gl_FragColor[0] = 0.4; "
        "  gl_FragColor[1] = 0.0; "
        "  gl_FragColor[2] = 0.0; "
        "}";
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
    if (!compile_ok) {
        fprintf(stderr, "!! Error in fragment shader \n");
        return EXIT_FAILURE;
    }


    //----- program
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "!! Error in glLinkProgram \n");
        return EXIT_FAILURE;
    }


    //----- attribute
    const char* attribute_name = "coord2d";
    attribute_coord2d = glGetAttribLocation(program, attribute_name);
    if (attribute_coord2d == -1) {
        fprintf(stderr, "!! Could not bind attribute %s \n", attribute_name);
        return EXIT_FAILURE;
    }


    //----- geometry data
    glGenBuffers(1, &vbo_lines);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
    glBufferData(GL_ARRAY_BUFFER, n_lines*4*sizeof(GLfloat), (void *)line_data, GL_STATIC_DRAW);

 //   glEnable(GL_PROGRAM_POINT_SIZE);
 
    return EXIT_SUCCESS;
}


void render(SDL_Window* window, uint32_t n_lines) {

    glClearColor(1.0, 1.0, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
    glEnableVertexAttribArray(attribute_coord2d);
    
    glVertexAttribPointer(
        attribute_coord2d, // attribute
        2,                 // number of elements per vertex, here (x,y)
        GL_FLOAT,          // the type of each element
        GL_FALSE,          // take our values as-is
        0,                 // no extra data between each position
        0                  // offset of first element
    );

    glDrawArrays(GL_LINES, 0, n_lines*2);
 
    glDisableVertexAttribArray(attribute_coord2d);

    SDL_GL_SwapWindow(window);
}


void free_graphical_resources() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_lines);
}

