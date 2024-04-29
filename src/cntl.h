#ifndef SRE_CONTROLABLE_H
#define SRE_CONTROLABLE_H

#include <cglm/mat4.h>
#include <stdbool.h>

typedef struct struct_sre_hcntl
{
    float yaw;
    float pitch;

    float speed;
    float sensitivity;
    vec3 orig;
    vec3 dir;
    vec3 up;
    vec3 right;

    unsigned char flags;
}
sre_cntl_handle;

typedef enum 
{
    CTL_DIR_UP = 1,
    CTL_DIR_DOWN = 2,
    CTL_DIR_BACK = 4,
    CTL_DIR_FRONT = 8,
    CTL_DIR_RIGHT = 16,
    CTL_DIR_LEFT = 32,
    CTL_MOUSE_MOVEMENT = 64,
}
ctl_dir_enum;
void SRE_Create_cntl(sre_cntl_handle *cntl, float yaw, float pitch, float speed, float sensitivity, vec3 orig, vec3 up);
void SRE_Create_cntl_indirect(sre_cntl_handle *src_cntl, sre_cntl_handle *dest_cntl);
void SRE_Cntl_translate(sre_cntl_handle *cntl, float dt);
void SRE_Cntl_rotate(sre_cntl_handle *cntl, float dt, float dx, float dy);

#endif