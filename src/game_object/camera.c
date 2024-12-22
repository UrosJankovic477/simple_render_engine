#include <sre/game_object/camera.h>
#include <sre/game_object/game_object.h>
#include <sre/game_object/group.h>
#include <math.h>

void SRE_Camera_create(sre_camera **camera, const char *name, vec3 translation, vec3 direction, float nearz, float farz, float asprat, float fovy)
{
    SRE_Game_object_create(name, (sre_game_object**)camera);
    (*camera)->base.go_type = SRE_GO_CAMERA;
    glmc_vec3_copy(translation, (*camera)->translation);
    glmc_vec3_normalize(direction);
    glmc_vec3_copy(direction, (*camera)->direction);
    glmc_vec3_copy((vec3){0.0f, 1.8f, 0.0f}, (*camera)->lookat_offset);
    (*camera)->asprat = asprat;
    (*camera)->farz = farz;
    (*camera)->nearz = nearz;
    (*camera)->fovy = fovy;
    (*camera)->proj_uniform = 0;
    (*camera)->view_uniform = 0;
    glm_mat4_identity((*camera)->view);
    glm_mat4_identity((*camera)->proj);
}

void SRE_Camera_set_view_mat(sre_camera *cam)
{
    vec3 translation;

    glmc_mat4_mulv3(cam->base.parent_transform_mat, cam->translation, 1.0f, translation);

    vec3 lookat;
    glmc_vec3_add(*cam->lookat, cam->lookat_offset, lookat);
    //glmc_look(translation, cam->direction, (vec3){0.0f, 1.0f, 0.0f}, cam->view);
    glmc_lookat(translation, lookat, (vec3){0.0f, 1.0f, 0.0f}, cam->view);
}

void SRE_Camera_set_proj_mat(sre_camera *cam)
{
    glmc_perspective(cam->fovy, cam->asprat, cam->nearz, cam->farz, cam->proj);
}
