#ifndef SRE_POSTPROC_H
#define SRE_POSTPROC_H

#include <glad/glad.h>
#include <stdlib.h>
#include <sre/shaders.h>

typedef struct struct_sre_framebuffer
{
    GLuint id;
    union
    {
        GLuint textures[2];
        struct maps {
            GLuint color_map;
            GLuint depth_map;
        } maps;
    };
    GLsizei width;
    GLsizei height;

}
sre_framebuffer;

extern GLuint postproc_plane_vao;
extern GLuint postproc_plane_vbo;
extern GLuint postproc_program;
extern GLuint postproc_vert_shdr;
extern GLuint postproc_frag_shdr;
extern GLint postproc_uniform_tex_id;
extern GLint postproc_uniform_time;

void SRE_Init_postproc();
void SRE_Delete_postproc_plane();
int SRE_Framebuffer_init(sre_framebuffer *framebuffer, GLsizei width, GLsizei height, GLenum fmt);
void SRE_Framebuffer_bind(sre_framebuffer framebuffer, GLuint program);
void SRE_Draw_to_main_framebuffer(sre_framebuffer framebuffer, GLuint postproc_prog, GLsizei width, GLsizei height);
void SRE_Delete_framebuffer(sre_framebuffer *framebuffer);


#endif
