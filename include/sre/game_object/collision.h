#ifndef SRE_COLLISION_H
#define SRE_COLLISION_H
#include <sre/types.h>
#include <sre/shaders.h>
#include <stdlib.h>
#include <cglm/call/mat4.h>
#include <cglm/call/vec4.h>
#include <cglm/call/vec3.h>
#include <cglm/call/vec2.h>
#include <glad/glad.h>
#include <sre/transform/transform.h>
#include <sre/game_object/game_object_base.h>
#include <sre/game_object/mesh.h>

#define SRE_MAX_NUM_COLS 128

typedef struct struct_sre_collider sre_collider;

typedef struct struct_sre_collision_event
{
   vec3 intersection;
   vec3 end;
   vec3 normal;
   sre_collider *other_collider;
   sre_collider *moving_collider;
   uint16_t bsp_tree_index;
   uint16_t bsp_tree_parent;
}
sre_collision_event;

typedef void (*sre_collision_handler)(sre_collision_event *event);

typedef enum enum_sre_collider_type {
   SRE_COLLIDER_NONE,
   SRE_COLLIDER_BSP_TREE,
   SRE_COLLIDER_AABB,
}
sre_collider_type;

typedef struct struct_sre_aabb
{
   vec3 min_pt;
   vec3 max_pt;
}
sre_aabb;

typedef struct struct_sre_bsp_node
{
   vec4 deviding_plane;
   uint16_t children[2];
}
sre_bsp_node;

typedef struct struct_sre_bsp_tree
{
   sre_bsp_node *tree;
   uint16_t size;
}
sre_bsp_tree;

typedef union union_sre_collider_data
{
   sre_bsp_tree bsp_tree;
   sre_aabb aabb;
}
sre_collider_data;

typedef struct struct_sre_collider
{
   sre_game_object_base base;
   sre_collider_type type;
   sre_collider_data data;
   sre_collision_handler handler;
}
sre_collider;

int SRE_Collider_create_aabb(sre_collider **collider, const char *name, vec3 min_pt, vec3 max_pt);
int SRE_Copy_collider(sre_collider *src, sre_collider *dest);

int SRE_Collider_get_next_in_queue(sre_collider **col);
int SRE_Collider_get_from_queue(uint8_t index, sre_collider **col);

bool SRE_Collider_test(sre_collider *col_1, sre_collider *col_2);
bool SRE_Collider_test_aabbs(sre_aabb *col_1, sre_aabb *col_2);
bool SRE_Collider_test_aabb_bsp_tree(sre_aabb *aabb, sre_bsp_node *bsp_tree);

void SRE_Collider_expand(sre_aabb moving_aabb, sre_collider *expanded_collider);
//void SRE_Collider_transform_handler(sre_collider_transform_event );
void SRE_Collider_trace_line(sre_transform_event *trans_event, sre_collider *collider, sre_collision_event *out_event);
void SRE_Collision_handler_solid(sre_collision_event *event);

int SRE_Collider_load(sre_collider *col);
void SRE_Collider_unload(sre_collider *col);

void SRE_Collider_translate(sre_collider *col, vec3 xyz);
void SRE_Collider_translate_aabb(sre_aabb *aabb, vec3 xyz);

bool SRE_Aabb_from_mesh(sre_mesh mesh, sre_collider **collider_out);
void SRE_Collider_get_center(sre_collider *col, vec3 center);

void SRE_Collider_draw(sre_collider *col, sre_program program);

#endif
