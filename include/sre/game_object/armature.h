#ifndef SRE_ARMATURE_H
#define SRE_ARMATURE_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <cglm/call/vec2.h>
#include <sre/types.h>
#include <sre/hashmap.h>
#include <sre/shaders.h>
#include <sre/game_object/game_object_base.h>

typedef struct struct_sre_keyframe
{
    uint32_t timestamp;
    mat4 *bone_matrices;
}
sre_keyframe;

typedef struct struct_sre_action
{
    char name[64];
    uint32_t bone_count;
    uint32_t keyframe_count;
    sre_keyframe *keyframes;
}
sre_action;

typedef struct struct_sre_armature
{
    sre_game_object_base base;
    uint16_t action_count;
    uint16_t bone_count;
    sre_action *actions;
}
sre_armature;

int SRE_Action_get_by_name(sre_armature *armature, const char *action_name, sre_action *action);
int SRE_Action_set_active(sre_action *action);
int SRE_Set_current_keyframes(sre_program program, uint32_t time);

#endif
