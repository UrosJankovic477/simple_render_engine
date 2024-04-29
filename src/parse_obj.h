#ifndef PARSE_OBJ_H
#define PARSE_OBJ_H
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashmap.h"
#include "texture.h"
#include "types.h"
#define SRE_MAX_TEXTURE_COORDINATES 8
#define SRE_MAX_INDECIES_COUNT 0x07ffffff
#define SRE_MAX_NUM_OF_MTLS 0xffff


typedef struct struct_sre_material
{
    char *name;
    unsigned short id;
    sre_rgba Ka;
    sre_rgba Kd;
    sre_rgba Ks;
    sre_rgba Ke;
    float Ns;
    sre_texture *map_Ka;
    sre_texture *map_Kd;
    sre_texture *map_Ks;
    sre_texture *map_Ns;
    sre_texture *map_Ke;
    struct struct_sre_material *collision;
}
sre_material;


typedef struct struct_sre_group_obj
{
    char *material_name;
    sre_material *material;
    int index;
}
sre_group_obj;

typedef struct struct_sre_mesh_obj
{
    sre_float_vec3 *position;
    sre_norm_16_vec2 *uv[SRE_MAX_TEXTURE_COORDINATES];
    sre_2_10_10_10s *normal;
    unsigned int position_count;
    unsigned int uv_count;
    unsigned int normals_count;
    unsigned int indecies_count;
    unsigned int *indecies[3];
    unsigned int group_count;
    sre_group_obj *groups;
}
sre_mesh_obj;

typedef struct struct_sre_model_obj
{
    unsigned int id;
    sre_mesh_obj **meshes;
    unsigned short mesh_count;
}
sre_model_obj;

typedef enum _sre_obj_token_enum{
    UNDEFINED,
    COMMENT,
    OBJECT,
    VERTEX_SPACE_COORDINATES,
    VERTEX_TEXTURE_COORDINATES,
    VERTEX_NORMALS,
    FACES,
    USE_MATERIAL,
    MATERIAL_LIBRARY,
    GROUP,
    LOD,
    MATERIAL,
    K_AMBIENT,
    K_DIFFUSE,
    K_SPECULAR,
    K_EMISSION,
    EXP_SPECULAR,
    MAP_AMBIENT,
    MAP_DIFFUSE,
    MAP_SPECULAR,
    MAP_SPECULAR_EXP,
    MAP_EMISSION,
}
sre_obj_token_enum;

typedef struct struct_sre_obj_token
{
    const char *key;
    char index;
    struct struct_sre_obj_token *collision;
    sre_obj_token_enum type;
}
sre_obj_token;

extern sre_obj_token *token_table_obj[32];
extern sre_obj_token *token_table_mtl[32];

#ifdef __cplusplus
extern "C" {
#endif



int add_token(sre_obj_token **token_table, const char *token, sre_obj_token_enum type);
unsigned char  get_token_hash(const char *token);
sre_obj_token* get_token(sre_obj_token **token_table, const char *token);


int add_material(sre_material* material);
sre_material* get_material(unsigned char *name);


void init_obj_parser();
void init_mtl_parser();
void init_parser();
void destroy_material_table();
void delete_material(sre_material *mtl);
void delete_model_obj(sre_model_obj *model);
void delete_mesh_obj(sre_mesh_obj *mesh);
    

int process_obj_file(const char *p_file_path, sre_model_obj *p_model);
int process_obj_mesh(FILE *p_file, 
    fpos_t *position, 
    sre_mesh_obj *p_mesh);
int procces_mtl_file(const char *p_file_path);
int load_mesh(sre_mesh_obj *p_mesh);
#ifdef __cplusplus
}
#endif
#endif