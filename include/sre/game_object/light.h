#ifndef SRE_LIGHT_H
#define SRE_LIGHT_H

#include <cglm/types.h>
#include <cglm/call/vec4.h>
#include <cglm/call/vec3.h>
#include <sre/types.h>
#include <sre/shaders.h>
#include <sre/transform/transform.h>
#include <sre/game_object/game_object_base.h>

typedef enum enum_sre_light_type
{
    SRE_POINT_LIGHT = 1,
    SRE_DIRECTIONAL_LIGHT = 2,
    SRE_SPOTLIGHT = 4,
}
sre_light_type;

typedef struct struct_sre_light
{
    sre_game_object_base base;
    vec3 translation;
    sre_rgba color;
    float radius;
    int idx;
    uint8_t type;
}
sre_light;

typedef struct struct_sre_spotlight
{
    sre_light light_base;
    vec4 rotation_quat;
    float spot_radius;
    float feather;
}
sre_spotlight;

#ifdef __cplusplus
extern "C" {
#endif

void SRE_Light_create(sre_light **light, const char *name, vec3 translation, sre_rgba color, float radius, bool directional);
void SRE_Light_push_uniform_array(sre_light *light, sre_program program);
void SRE_Light_put_unifrorm_array(sre_light light, sre_program program);
void SRE_Light_set_position_uniform(sre_light light, sre_program program);
void SRE_Light_set_color_uniform(sre_light light, sre_program program);
void SRE_Light_set_radius_uniform(sre_light light, sre_program program);
void SRE_Light_transform(sre_light *light, mat4 transform);
void SRE_Light_draw(sre_light *light, sre_program program);

#ifdef __cplusplus
}
#endif

#endif
