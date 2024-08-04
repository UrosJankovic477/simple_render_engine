#ifndef SRE_COLLISION_H
#define SRE_COLLISION_H
#include "types.h"
#include "shaders.h"
#include <stdlib.h>
#include <cglm/mat4.h>
#include <cglm/vec4.h>
#include <cglm/vec3.h>
#include <cglm/vec2.h>
#include <glad/glad.h>

#define SRE_MAX_NUM_OF_COLS 128

typedef enum enum_sre_collider_type {
   COL_NONE,
   COL_BSP_TREE,
   COL_AABB,
}
sre_collider_type;

typedef struct struct_sre_collider {
    sre_collider_type type;
    int col_queue_idx;
    void *data;
}
sre_collider;

typedef struct struct_sre_coldat_aabb
{
   vec3 min_pt;
   vec3 max_pt;
}
sre_coldat_aabb;

typedef struct struct_sre_coldat_bsp_tree
{
   vec4 deviding_plane;
   uint16_t children[2];
}
sre_coldat_bsp_tree;

typedef struct struct_sre_collision_context
{
   vec3 intersection;
   vec3 end;
   vec3 normal;
   sre_collider_type type;
   void *coldat;
   sre_coldat_aabb *moving_collider;
   uint16_t bsp_tree_index;
   uint16_t bsp_tree_parent;
}
sre_collision_context;

extern sre_collider *col_queue[SRE_MAX_NUM_OF_COLS];
extern uint8_t col_count;


int SRE_Create_collider(sre_collider *col, void *data, sre_collider_type type);
int SRE_Copy_collider(sre_collider *src, sre_collider *dest);

bool SRE_Col_test(sre_collider *col_1, sre_collider *col_2);
bool SRE_Col_test_aabbs(sre_coldat_aabb *col_1, sre_coldat_aabb *col_2);
bool SRE_Col_test_aabb_bsp_tree(sre_coldat_aabb *aabb, sre_coldat_bsp_tree *bsp_tree);

void SRE_Expand_collision(sre_coldat_aabb *coldat_moving, sre_coldat_aabb *coldat_static, sre_coldat_aabb *coldat_expanded);
bool SRE_Line_intersection(vec3 start, sre_collision_context *context, void (*col_handler)(sre_collision_context *, void *));
void SRE_Col_handler_solid(sre_collision_context *context, void *args);

bool SRE_Col_load(sre_collider *col);
void SRE_Col_unload(sre_collider *col);

void SRE_Col_translate(sre_collider *col, vec3 xyz);
void SRE_Col_translate_aabb(sre_coldat_aabb *aabb, vec3 xyz);

bool SRE_Create_mesh_boudning_box(sre_collider *col_out, vec3 *coords, unsigned int n);
void SRE_Col_get_center(sre_collider *col, vec3 center);

void SRE_Draw_collider(sre_collider *col, sre_program program);

#endif