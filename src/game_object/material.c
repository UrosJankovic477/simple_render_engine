#include <sre/game_object/material.h>

int SRE_Material_bind(sre_material *mtl, sre_program program)
{
    glUniform1ui(SRE_Get_uniform_location(&program, "ks"), mtl->Ks);
    glUniform1ui(SRE_Get_uniform_location(&program, "ns"), mtl->Ns);

    if (mtl->map_Kd != NULL)
    {
        glBindTexture(GL_TEXTURE_2D, mtl->map_Kd->id);
        glUniform1ui(SRE_Get_uniform_location(&program, "use_diffuse_tex"), GL_TRUE);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1ui(SRE_Get_uniform_location(&program, "use_diffuse_tex"), GL_FALSE);
        glUniform1ui(SRE_Get_uniform_location(&program, "kd"), mtl->Kd.rgba);
    }
    return SRE_SUCCESS;
}
