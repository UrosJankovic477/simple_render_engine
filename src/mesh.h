#ifndef SRE_MESH_H
#define SRE_MESH_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "types.h"
#ifndef SRE_MATERIAL_HANDLE_T
#define SRE_MATERIAL_HANDLE_T uint64_t
#endif
#ifndef SRE_ARMATURE_HANDLE_T
#define SRE_ARMATURE_HANDLE_T uint64_t
#endif

typedef struct struct_sre_mesh_data
{
    uint64_t vertex_count;
    sre_float_vec3 *vertex_positions;
    sre_2_10_10_10s *vertex_normals;
    uint64_t index_count;
    uint64_t *vertex_indices;
    sre_norm_16_vec2 *uv_coordinates;
    sre_2_10_10_10s *polygon_normals;
    sre_rgba *vertex_colors;
    SRE_MATERIAL_HANDLE_T material_handle;
    SRE_ARMATURE_HANDLE_T armature_handle;
} sre_mesh_data;

typedef struct struct_sre_vertex_groups_data
{
    uint8_t group_count;
    uint8_t *groups;
    float *values;
} sre_vertex_groups_data;

int SRE_Mesh_import(const char *filepath, sre_mesh_data *mesh_data);

#endif