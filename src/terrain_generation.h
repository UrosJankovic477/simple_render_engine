#ifndef SRE_TERRAIN_GENERATION_H
#define SRE_TERRAIN_GENERATION_H

#include <glad/glad.h>
#include <cglm/mat4.h>
#include "types.h"
#include "shaders.h"

typedef struct struct_sre_terrain
{
    uint32_t w;
    uint32_t h;
    sre_vtx_float_float_2_10_10_10 *vertex_data;
    uint32_t *indicies;
    int32_t *basevertex;
    GLuint vao;
    GLuint vbo;
}
sre_terrain;


int generate_terrain_vertices(uint32_t w, uint32_t h, sre_terrain *out_terrain, unsigned int tex_rep);
void draw_terrain(sre_terrain terrain_data, sre_program program);
void delete_terrain_vertices(sre_terrain *terrain_data);

#endif