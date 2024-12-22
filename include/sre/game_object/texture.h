#ifndef SRE_TEXTURE_H
#define SRE_TEXTURE_H
#include <glad/glad.h>
#include <stb/stb_image.h>
#include <sre/image.h>
#include <sre/types.h>
#include <sre/logging/errors.h>
#include <sre/game_object/game_object_base.h>

typedef struct struct_sre_texture
{
    sre_game_object_base base;
    GLuint id;
    GLint width;
    GLint height;
    GLint nChannels;
    GLubyte *data;
    char *path; 
}
sre_texture;

#ifdef __cplusplus
extern "C" {
#endif

int SRE_Load_texture(const char *path, sre_texture *texture_out);

int SRE_Delete_texture(sre_texture texture);

#ifdef __cplusplus
}
#endif

#endif