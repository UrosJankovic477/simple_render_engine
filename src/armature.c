#include "armature.h"

sre_action *active_action = NULL;
uint8_t kf1_idx = 0;
uint8_t kf2_idx = 1;

int SRE_Armature_process(sre_importer *importer, sre_mempool *asset_mempool, FILE *file, fpos_t *position, sre_armature *armature)
{
    char buffer[1028];
    int64_t action_idx = -1, kf_idx = -1;
    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, blank_chars);
        sre_keyword keyword;
        SRE_Get_keyword(importer, token, &keyword);
        switch (keyword)
        {
            case SRE_ACT_COUNT:
            {
                armature->action_count = atoi(strtok(NULL, blank_chars));
                int status = SRE_Mempool_alloc(asset_mempool, (void**)&armature->actions, armature->action_count * sizeof(sre_action));
                if (status != SRE_SUCCESS)
                {
                    return SRE_ERROR;
                }
                
                break;
            }
            case SRE_BONE_COUNT:
            {
                armature->bone_count = atoi(strtok(NULL, blank_chars));
                break;
            }
            case SRE_ACT:
                action_idx += 1;
                kf_idx = -1;
                const char *name = strtok(NULL, blank_chars);
                strcpy_s(armature->actions[action_idx].name, 64, name);
                armature->actions[action_idx].bone_count = armature->bone_count;
                break;
            case SRE_KF_COUNT:
            {
                armature->actions[action_idx].keyframe_count = atoi(strtok(NULL, blank_chars));
                int status = SRE_Mempool_alloc(asset_mempool, (void**)&armature->actions[action_idx].keyframes, armature->actions[action_idx].keyframe_count * sizeof(sre_keyframe));
                if (status != SRE_SUCCESS)
                {
                    return SRE_ERROR;
                }
                break;
            }
            case SRE_KF:
            {

                kf_idx += 1;
                armature->actions[action_idx].keyframes[kf_idx].timestamp = atoi(strtok(NULL, blank_chars));
                int status = SRE_Mempool_alloc(asset_mempool, (void**)&armature->actions[action_idx].keyframes[kf_idx].bone_matrices, armature->bone_count * 16 * sizeof(float));
                if (status != SRE_SUCCESS)
                {
                    return SRE_ERROR;
                }
                for (size_t bone_idx = 0; bone_idx < armature->bone_count; bone_idx++)
                {
                    fgets(buffer, sizeof(buffer), file);
                    strtok(buffer, blank_chars);
                    for (size_t i = 0; i < 4; i++)
                    {
                        for (size_t j = 0; j < 4; j++)
                        {
                            armature->actions[action_idx].keyframes[kf_idx].bone_matrices[bone_idx][i][j] = atof(strtok(NULL, blank_chars));
                        }
                    }   
                }
                break;
            }
            case SRE_ENDOBJ:
                break;
            default:
                break;
        }
    }
    return SRE_SUCCESS;
}

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
    uint32_t action_time = (time / 500000) % action_duration;
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
    
    printf("%f\n", t_normalized);
    glUniform1f(SRE_Get_uniform_location(&program, "t_normalized"), t_normalized);
    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "bone_matrices_kf1"), active_action->bone_count, GL_FALSE, &active_action->keyframes[kf1_idx].bone_matrices[0][0][0]);
    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "bone_matrices_kf2"), active_action->bone_count, GL_FALSE, &active_action->keyframes[kf2_idx].bone_matrices[0][0][0]);
    return SRE_SUCCESS;
}
