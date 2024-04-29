#ifndef SRE_PROPS_H
#define SRE_PROPS_H
#include <cglm/affine.h>
#include <cglm/euler.h>
#include "shaders.h"
#include "model.h"
#include "collision.h"
#include "cntl.h"


typedef struct struct_sre_prop {
    gl_mesh *mesh;
    sre_collider *collider;
    sre_cntl_handle *cntl;

    mat4 model;
}
sre_prop;

void create_prop(gl_mesh *mesh, sre_collider *collider, mat4 *model, sre_prop *out_prop, sre_cntl_handle *cntl);
void draw_prop(sre_prop prop, sre_program program);
void translate_prop(sre_prop *prop, vec3 xyz);

void rotate_prop(sre_prop *prop, vec3 axis, float angle);
void rotate_prop_x(sre_prop *prop, float angle);
void rotate_prop_y(sre_prop *prop, float angle);
void rotate_prop_z(sre_prop *prop, float angle);

void rotate_euler_prop(sre_prop *prop, vec3 euler_rot);

void scale_prop(sre_prop *prop, vec3 xyz);

void get_min_aabb(sre_prop *prop, sre_coldat_aabb *data);

void prop_control(sre_prop *prop, float dt, float dx, float dy);

#endif
