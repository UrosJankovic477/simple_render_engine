#ifndef SRE_PROPS_H
#define SRE_PROPS_H
#include <cglm/affine.h>
#include <cglm/euler.h>
#include "shaders.h"
#include "mesh.h"
#include "collision.h"
#include "cntl.h"


typedef struct struct_sre_prop {
    sre_mesh *mesh;
    sre_collider *collider;
    sre_cntl_handle *cntl;
}
sre_prop;

void SRE_Prop_create(sre_mesh *mesh, sre_collider *collider, sre_prop *out_prop, sre_cntl_handle *cntl);
void SRE_Prop_draw(sre_prop prop, sre_program program, bool draw_collider);
void SRE_Prop_translate(sre_prop *prop, vec3 xyz);

void SRE_Prop_rotate(sre_prop *prop, vec3 axis, float angle);
void SRE_Prop_rotate_x(sre_prop *prop, float angle);
void SRE_Prop_rotate_y(sre_prop *prop, float angle);
void SRE_Prop_rotate_z(sre_prop *prop, float angle);

void SRE_Prop_rotate_euler(sre_prop *prop, vec3 euler_rot);

void SRE_Prop_scale(sre_prop *prop, vec3 xyz);

void SRE_Prop_gen_aabb(sre_prop *prop, sre_mempool *mempool);

void SRE_Prop_control(sre_prop *prop, float dt, float dx, float dy);


#endif
