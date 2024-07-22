#ifndef SRE_MESH_H
#define SRE_MESH_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <glad/glad.h>
#include <cglm/vec4.h>
#include <cglm/ivec4.h>
#include "material.h"
#include "armature.h"
#include "types.h"
#include "mem_allocation.h"
#include "importer.h"
#include "hashmap.h"
#include "shaders.h"

#define MAX_VERTEX_COUNT 0xffff

typedef struct struct_sre_mesh
{
    mat4 model_mat;
    GLuint vao;
    GLuint vbo;
    uint64_t vertex_count;
    sre_float_vec3 *vertex_positions;
    sre_2_10_10_10s *vertex_normals;
    uint64_t index_count;
    uint64_t *vertex_indices;
    sre_norm_16_vec2 *uv_coordinates;
    sre_2_10_10_10s *polygon_normals;
    sre_rgba *vertex_colors;
    vec4 *weights;
    ivec4 *bones;
    sre_material *material;
    sre_armature *armature;
    char *material_name;
    char *armature_name;
} sre_mesh;

int SRE_Mesh_process(sre_importer *importer, sre_mempool *asset_mempool, FILE *file, fpos_t *position, sre_mesh *mesh);
int SRE_Mesh_load(sre_mesh *mesh);
int SRE_Mesh_unload(sre_mesh *mesh);
int SRE_Mesh_draw(sre_mesh *mesh, sre_program program);

#endif