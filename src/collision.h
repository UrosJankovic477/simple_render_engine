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
   COL_NONE = 0,
   COL_SPHERE = 0x1,
   COL_CAPSULE = 0x2,
   COL_RECT = 0x20,
   COL_AABB = 0x40,
}
sre_collider_type;

typedef struct struct_sre_collider {
    sre_collider_type type;
    int col_queue_idx;
    void *data;
}
sre_collider;

typedef struct struct_sre_coldat_sphere
{
   vec3 center;
   float r;
}
sre_coldat_sphere;

typedef struct struct_sre_coldat_capsule
{
   vec2 center;
   float ymin;
   float ymax;
   float r;
}
sre_coldat_capsule;

typedef struct struct_sre_coldat_rect
{
   vec3 normal;
   float offset;
}
sre_coldat_rect;

typedef struct struct_sre_coldat_aabb
{
   vec3 min_pt;
   vec3 max_pt;
}
sre_coldat_aabb;

extern sre_collider *col_queue[SRE_MAX_NUM_OF_COLS];
extern uint8_t col_count;


int SRE_Create_collider(sre_collider *col, void *data, sre_collider_type type);
int SRE_Copy_collider(sre_collider *src, sre_collider *dest);

bool SRE_Col_test(sre_collider *col_1, sre_collider *col_2);
bool SRE_Col_test_spheres(sre_coldat_sphere *col_1, sre_coldat_sphere *col_2);
bool SRE_Col_test_aabbs(sre_coldat_aabb *col_1, sre_coldat_aabb *col_2);
bool SRE_Col_test_capsules(sre_coldat_capsule *col_1, sre_coldat_capsule *col_2);
bool SRE_Col_test_capsule_sphere(sre_coldat_capsule *capsule, sre_coldat_sphere *sphere);
bool SRE_Col_test_aabb_sphere(sre_coldat_aabb *aabb, sre_coldat_sphere *sphere);
bool SRE_Col_test_capsule_aabb(sre_coldat_capsule *capsule, sre_coldat_aabb *aabb);

void SRE_Expand_collision(sre_collider *moving_col, sre_collider *static_col, sre_collider *expanded_col);
//void expand_sphere_with_sphere(sre_collider *moving_col, sre_collider *static_col, sre_collider *expanded_col);
//void expand_capsule_with_sphere(sre_collider *moving_col, sre_collider *static_col, sre_collider *expanded_col);

bool SRE_Line_intersection(vec3 start, vec3 end, sre_collider *col, vec3 intersection);

bool SRE_Col_load(sre_collider *col);
void SRE_Col_unload(sre_collider *col);

void SRE_Col_translate(sre_collider *col, vec3 xyz);
void SRE_Col_translate_capsule(sre_coldat_capsule *capsule, vec3 xyz);
void SRE_Col_translate_sphere(sre_coldat_sphere *sphere, vec3 xyz);
void SRE_Col_translate_aabb(sre_coldat_aabb *aabb, vec3 xyz);

bool SRE_Create_mbb(sre_collider *col_out, vec3 *coords, unsigned int n);
void SRE_Col_get_center(sre_collider *col, vec3 center);

void SRE_Draw_collider(sre_collider *col, sre_program program);

#endif