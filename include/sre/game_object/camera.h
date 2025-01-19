#ifndef SRE_CAMERA_H
#define SRE_CAMERA_H

#define _USE_MATH_DEFINES
#include <math.h>

#include <cglm/call/cam.h>
#include <cglm/types.h>
#include <cglm/call/mat4.h>
#include <cglm/call/affine.h>
#include <sre/game_object/game_object_base.h>
#include <sre/game_object/cntl.h>

typedef struct struct_sre_camera
{
    sre_game_object_base base;
    GLuint view_uniform;
    GLuint proj_uniform;
    mat4 view;
    mat4 proj;
    vec3 translation;
    vec3 direction;
    char lookat_name[64];
    sre_group *lookat;
    vec3 lookat_offset;
    float fovy;
    float nearz;
    float farz;
    float asprat;
}
sre_camera;

extern sre_camera *main_camera;

#ifdef __cplusplus
extern "C" {
#endif

void SRE_Camera_create(sre_camera **camera, const char *name, vec3 translation, vec3 direction, float nearz, float farz, float asprat, float fovy);
void SRE_Camera_transform(sre_camera *cam, sre_transform transform);
void SRE_Camera_set_view_mat(sre_camera *cam);
void SRE_Camera_set_proj_mat(sre_camera *cam);


#ifdef __cplusplus
}
#endif

#endif
