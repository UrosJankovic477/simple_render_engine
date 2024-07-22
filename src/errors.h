#ifndef SRE_ERRORS_H
#define SRE_ERRORS_H
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>

#define SRE_SUCCESS 0
#define SRE_ERROR 1
#define SRE_ERROR_NO_MEM 2

void read_gl_err_queue();

void GLAPIENTRY MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam );


#endif