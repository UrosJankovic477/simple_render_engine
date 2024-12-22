#include <sre/game_object/armature.h>
#include <sre/logging/errors.h>
#include <SDL2/SDL.h>

sre_action *active_action = NULL;
uint8_t kf1_idx = 0;
uint8_t kf2_idx = 1;

int SRE_Action_get_by_name(sre_armature *armature, const char *action_name, sre_action *action)
{
    for (size_t i = 0; i < armature->action_count; i++)
    {
        if (strcmp(action_name, armature->actions[i].name) == 0)
        {
            *action = armature->actions[i];
            return SRE_SUCCESS;
        }
    }

    return SRE_ERROR;
}

int SRE_Action_set_active(sre_action *action)
{
    active_action = action;
    kf1_idx = 0;
    kf2_idx = 1;
    return SRE_SUCCESS;
}

int SRE_Set_current_keyframes(sre_program program, uint32_t time)
{
    glUniform1ui(SRE_Get_uniform_location(&program, "animated"), true);
    uint32_t t1 = active_action->keyframes[kf1_idx].timestamp;
    uint32_t t2 = active_action->keyframes[kf2_idx].timestamp;
    uint32_t action_duration = active_action->keyframes[active_action->keyframe_count - 1].timestamp;
    uint32_t action_time = (time >> 24) % action_duration;
    if (action_time <= active_action->keyframes[1].timestamp)
    {
        kf1_idx = 0;
        kf2_idx = 1;
    }
    else if (action_time > t2)
    {
        kf1_idx = kf2_idx;
        kf2_idx = (kf2_idx + 1);
        if (kf2_idx == active_action->keyframe_count)
        {
            kf2_idx -= 1;
        }

    }
    t1 = active_action->keyframes[kf1_idx].timestamp;
    t2 = active_action->keyframes[kf2_idx].timestamp;
    float kf_matrices;
    float dt = (float)(t2 - t1);
    float t_normalized = ((float)(action_time - t1)) / (float)dt;
    if (t_normalized > 1.0f)
    {
        t_normalized = 0.0f;
    }

    glUniform1f(SRE_Get_uniform_location(&program, "t_normalized"), t_normalized);
    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "bone_matrices_kf1"), active_action->bone_count, GL_FALSE, &active_action->keyframes[kf1_idx].bone_matrices[0][0][0]);
    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "bone_matrices_kf2"), active_action->bone_count, GL_FALSE, &active_action->keyframes[kf2_idx].bone_matrices[0][0][0]);
    return SRE_SUCCESS;
}
