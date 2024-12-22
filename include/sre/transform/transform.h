#ifndef SRE_TRANSFORM_H
#define SRE_TRANSFORM_H

#include <cglm/call/affine.h>
#include <cglm/call/euler.h>
#include <cglm/call/quat.h>
#include <cglm/call/vec3.h>
#include <cglm/call/vec4.h>
#include <sre/types.h>

typedef enum enum_sre_transform_flags
{
    SRE_TRANS_TRANSLATE_X = 1,
    SRE_TRANS_TRANSLATE_Y = 1 << 1,
    SRE_TRANS_TRANSLATE_Z = 1 << 2,
    SRE_TRANS_ROTATE_X = 1 << 3,
    SRE_TRANS_ROTATE_Y = 1 << 4,
    SRE_TRANS_ROTATE_Z = 1 << 5,
    SRE_TRANS_SCALE_X = 1 << 6,
    SRE_TRANS_SCALE_Y = 1 << 7,
    SRE_TRANS_SCALE_Z = 1 << 8,
    SRE_TRANS_CHECK_COLLISION = 1 << 9,
}
sre_transform_flags;

#define SRE_TRANS_TRANSLATE (SRE_TRANS_TRANSLATE_X | SRE_TRANS_TRANSLATE_Y | SRE_TRANS_TRANSLATE_Z)
#define SRE_TRANS_ROTATE (SRE_TRANS_ROTATE_X | SRE_TRANS_ROTATE_Y | SRE_TRANS_ROTATE_Z)
#define SRE_TRANS_SCALE (SRE_TRANS_SCALE_X | SRE_TRANS_SCALE_Y | SRE_TRANS_SCALE_Z)

typedef struct struct_sre_transform_euler
{
    vec3 translation;
    sre_euler rotation;
    vec3 scale;
}
sre_transform, sre_transform_euler;

typedef struct struct_sre_transform_quat
{
    vec3 translation;
    versor rotation_quat;
    vec3 scale;
}
sre_transform_quat;

typedef struct struct_sre_transform_constraint
{
    vec3 min;
    vec3 max;
}
sre_transform_constraint, sre_translation_constraint, sre_rotation_constraint, sre_scale_constraint;

typedef struct struct_sre_transform_event
{
    sre_transform start_trans;
    sre_transform end_trans;
    uint16_t trans_flags;
}
sre_transform_event;

typedef struct struct_sre_orientation
{
    vec4 dir;
    vec4 right;
    vec4 up;
}
sre_orientation;

void SRE_Orientation_update(sre_orientation *orientation, sre_euler angles);
void SRE_Orientation_create(sre_orientation *orientation);

void SRE_Trans_identity(sre_transform *trans);
void SRE_Trans_create_mat(sre_transform *trans, mat4 mat);
void SRE_Trans_copy(sre_transform src, sre_transform *dest);
void SRE_Trans_translate(sre_transform *trans, vec3 xyz);
void SRE_Trans_rotate_euler(sre_transform *trans, vec3 xyz);
void SRE_Trans_rotate_euler_x(sre_transform *trans, float x);
void SRE_Trans_rotate_euler_y(sre_transform *trans, float y);
void SRE_Trans_rotate_euler_z(sre_transform *trans, float z);
void SRE_Trans_scale(sre_transform *trans, vec3 xyz);
void SRE_Mat_transform(sre_transform *trans, mat4 mat);


#endif //SRE_TRANSFORM_H
