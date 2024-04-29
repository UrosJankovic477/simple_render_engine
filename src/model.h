#ifndef GL_MODEL_H
#define GL_MODEL_H


#include "parse_obj.h"
#include "types.h"
#include "shaders.h"
#include <glad/glad.h>
#include <cglm/types.h>
#include <cglm/mat4.h>


typedef struct struct_gl_mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint vertex_count;

    unsigned int material_count;
    sre_material **materials;
    unsigned int *group_indecies;
}
gl_mesh;


#ifdef __cplusplus
extern "C" {
#endif


int SRE_Load_gl_mesh(sre_mesh_obj *p_mesh, gl_mesh *p_gl_mesh);
int SRE_Draw_gl_mesh(gl_mesh *p_gl_mesh, sre_program program, mat4 model);
void SRE_Delete_gl_mesh(gl_mesh *mesh);


#ifdef __cplusplus
}
#endif


#endif