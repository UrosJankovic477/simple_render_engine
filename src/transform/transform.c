#include <sre/transform/transform.h>
#include <cglm/call/mat4.h>
#include <cglm/affine.h>

void SRE_Trans_identity(sre_transform *trans)
{
    glmc_vec3_zero(trans->translation);
    glmc_vec3_zero(trans->rotation.vector);
    glmc_vec3_one(trans->scale);
}

void SRE_Trans_create_mat(sre_transform *trans, mat4 mat)
{
    glmc_euler_yxz(trans->rotation.vector, mat);
    glmc_vec4(trans->translation, 1.0f, mat[3]);
    glmc_scale(mat, trans->scale);
}

void SRE_Trans_copy(sre_transform src, sre_transform *dest)
{
    glmc_vec3_copy(src.rotation.vector, dest->rotation.vector);
    glmc_vec3_copy(src.scale, dest->scale);
    glmc_vec3_copy(src.translation, dest->translation);
}

void SRE_Trans_translate(sre_transform *trans, vec3 xyz)
{
    glmc_vec3_add(trans->translation, xyz, trans->translation);
}

void SRE_Trans_rotate_euler(sre_transform *trans, vec3 xyz)
{
    glmc_vec3_add(trans->rotation.vector, xyz, trans->rotation.vector);
}

void SRE_Trans_rotate_euler_x(sre_transform *trans, float x)
{
    trans->rotation.vector[0] += x;
}

void SRE_Trans_rotate_euler_y(sre_transform *trans, float y)
{
    trans->rotation.vector[1] += y;
}

void SRE_Trans_rotate_euler_z(sre_transform *trans, float z)
{
    trans->rotation.vector[2] += z;
}

void SRE_Trans_scale(sre_transform *trans, vec3 xyz)
{
    glmc_vec3_mul(trans->scale, xyz, trans->scale);
}

void SRE_Mat_transform(sre_transform *trans, mat4 mat)
{
    mat4 trans_mat;
    SRE_Trans_create_mat(trans, trans_mat);
    glmc_mat4_mul(trans_mat, mat, mat);
}

void SRE_Orientation_update(sre_orientation *orientation, sre_euler angles)
{
    mat4 rotation;
    vec3 euler_angles;
    glmc_vec3_negate_to(angles.vector, euler_angles);
    glmc_euler_yxz(euler_angles, rotation);
    glmc_mat4_mulv3(rotation, (vec3){0.0f, 0.0f, 1.0f}, 1.0f, orientation->dir);
    glmc_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, orientation->up);
    glmc_vec3_cross(orientation->up, orientation->dir, orientation->right);
}

void SRE_Orientation_create(sre_orientation *orientation)
{
    glmc_vec3_copy((vec3){0.0f, 0.0f, 1.0f}, orientation->dir);
    glmc_vec3_copy((vec3){-1.0f, 0.0f, 0.0f}, orientation->right);
    glmc_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, orientation->up);
}
