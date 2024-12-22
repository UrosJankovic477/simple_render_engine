#ifndef GL_RENDER_PIPELINE_H
#define GL_RENDER_PIPELINE_H


#include <glad/glad.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sre/hashmap.h>

#define GL_SHADER_COMPILED_TRUE 1
#define GL_SHADER_COMPILED_FALSE 0
#define SH_MAX_UNIFORM_NUM 0xff


#ifdef __cplusplus
extern "C" {
#endif

typedef struct sre_uniform_struct
{
    GLint location;
    const char *name;
}
sre_uniform;

typedef struct sre_program_struct
{
    GLuint id;
    GLuint vertex_shader;
    GLuint fragment_shader;
    unsigned int light_count;
    unsigned int inf_light_count;
    sre_uniform uniform_cache[SH_MAX_UNIFORM_NUM];
    int uniforms_count;
}
sre_program;



int SRE_Compile_shader(const char *p_shader_path, GLenum p_shader_type, GLuint *p_shader_obj);
int SRE_Create_shader_program(sre_program *program, GLuint vertex_shader, GLuint fragment_shader);
int SRE_Get_uniform_location(sre_program *program, const char *uniform_name);
void SRE_Delete_shader_program(sre_program *program);

#ifdef __cplusplus
}
#endif


#endif