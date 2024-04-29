#ifndef SRE_CAMERA_H
#define SRE_CAMERA_H

#define _USE_MATH_DEFINES
#include <math.h>

#include <cglm/cam.h>
#include <cglm/types.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>

#include "cntl.h"

typedef struct sre_camera_struct
{
    float fovy;
    float nearz;
    float farz;
    float asprat;

    mat4 view;
    mat4 proj;

    sre_cntl_handle *cntl;
} sre_camera;

#ifdef __cplusplus
extern "C" {
#endif

//void cam_init_default(sre_camera *cam);
void SRE_Cam_init(sre_camera *cam, sre_cntl_handle *cntl, float nearz, float farz, float asprat, float fovy);

void SRE_Cam_set_view_mat(sre_camera *cam);

void SRE_Cam_set_proj_mat(sre_camera *cam);

void SRE_Cam_update_pos(sre_camera *cam, float dt, float dx, float dy);

#ifdef __cplusplus
}
#endif

#endif
