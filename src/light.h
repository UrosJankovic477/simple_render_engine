#ifndef SRE_LIGHT_H
#define SRE_LIGHT_H

#include <cglm/types.h>
#include <cglm/vec4.h>
#include <cglm/vec3.h>
#include "types.h"
#include "shaders.h"

typedef struct struct_sre_light
{
    vec4 position;
    sre_rgba color;
    float radius;
    int idx;
}
sre_light;



#ifdef __cplusplus
extern "C" {
#endif

void SRE_Light_push_uniform_array(sre_light *light, sre_program program);
void SRE_Light_put_unifrorm_array(sre_light light, sre_program program);
void SRE_Light_set_position_uniform(sre_light light, sre_program program);
void SRE_Light_set_color_uniform(sre_light light, sre_program program);
void SRE_Light_set_radius_uniform(sre_light light, sre_program program);
void SRE_Debug_draw_light(sre_light light, sre_program program);

#ifdef __cplusplus
}
#endif

#endif