#ifndef SRE_MESH_H
#define SRE_MESH_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <glad/glad.h>
#include <cglm/call/vec4.h>
#include <cglm/call/ivec4.h>
#include <sre/game_object/material.h>
#include <sre/game_object/armature.h>
#include <sre/types.h>
#include <sre/mem_allocation.h>
#include <sre/hashmap.h>
#include <sre/shaders.h>
#include <sre/transform/transform.h>
#include <sre/game_object/game_object_base.h>

#define MAX_VERTEX_COUNT 0xffff

typedef struct struct_sre_mesh
{
    sre_game_object_base base;
    uint32_t loaded_incatnce_count;
    mat4 model_mat;
    GLuint vao;
    GLuint vbo;
    uint32_t vertex_count;
    vec3 *vertex_positions;
    sre_2_10_10_10s *vertex_normals;
    uint32_t index_count;
    uint32_t *vertex_indices;
    sre_norm_16_vec2 *uv_coordinates;
    sre_2_10_10_10s *polygon_normals;
    sre_rgba *vertex_colors;
    vec4 *weights;
    ivec4 *bones;
    sre_material *material;
    sre_armature *armature;
    char material_name[64];
    char armature_name[64];
    char material_path[256];
    char armature_path[256];
} 
sre_mesh;

void SRE_Mesh_transform(sre_mesh *mesh, mat4 transform);
int SRE_Mesh_load(sre_mesh *mesh);
int SRE_Mesh_unload(sre_mesh *mesh);
int SRE_Mesh_draw(sre_mesh *mesh, sre_program *program);

#endif
