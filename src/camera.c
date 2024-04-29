#include "camera.h"

void SRE_Cam_init(sre_camera *cam, sre_cntl_handle *cntl, float nearz, float farz, float asprat, float fovy)
{
    cam->cntl = cntl;
    cam->asprat = asprat;
    cam->farz = farz;
    cam->nearz = nearz;
    cam->fovy = fovy;

    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam->cntl->up);
}


void SRE_Cam_set_view_mat(sre_camera *cam)
{
    glm_look(cam->cntl->orig, cam->cntl->dir, cam->cntl->up, cam->view);   
}

void SRE_Cam_set_proj_mat(sre_camera *cam)
{
    glm_perspective(cam->fovy, cam->asprat, cam->nearz, cam->farz, cam->proj);
}

void SRE_Cam_update_pos(sre_camera *cam, float dt, float dx, float dy)
{
    SRE_Cntl_rotate(cam->cntl, dt, dx, dy);
    SRE_Cntl_translate(cam->cntl, dt);
}
