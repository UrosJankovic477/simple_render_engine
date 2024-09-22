#include "material.h"

int SRE_Material_process(sre_importer *importer, sre_mempool *asset_mempool, FILE *file, fpos_t *position, sre_material *material)
{
    char buffer[1028];
    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, blank_chars);
        sre_keyword keyword;
        SRE_Get_keyword(importer, token, &keyword);
        switch (keyword)
        {
            case SRE_DIFFUSE_TEX:
                char *texture_name = strtok(NULL, blank_chars);
                sre_object texture_object;
                int status = SRE_Importer_get_object(importer, texture_name, &texture_object);
                if (status != SRE_SUCCESS)
                {
                    strncpy(texture_object.name, texture_name, 64);
                    texture_object.mempool = asset_mempool;
                    SRE_Mempool_alloc(asset_mempool, &texture_object.object, sizeof(sre_texture));
                    const char *extension = ".png";
                    char texture_path[256] = "..\\resources\\textures\\";
                    strncat(texture_path, texture_name, 256);
                    strncat(texture_path, extension, 256);
                    int result = SRE_Load_texture(texture_path, (sre_texture*)texture_object.object);
                    if (result != SRE_SUCCESS)
                    {
                        return result;
                    }
                    
                    SRE_Importer_add_object(importer, texture_object);
                }
                
                material->map_Kd = (sre_texture*)texture_object.object;
                break;
            case SRE_DIFFUSE:
            {

                float r = atof(strtok(NULL, blank_chars));
                float g = atof(strtok(NULL, blank_chars));
                float b = atof(strtok(NULL, blank_chars));
                material->Kd = SRE_Vec3_to_rgb(r, g, b);
                break;
            }
            case SRE_SPECULAR:
            {
                float ks = atof(strtok(NULL, blank_chars));
                material->Ks = SRE_Float_to_unorm_8(ks);
                break;
            }
            case SRE_EMISSION:
            {
                float r = atof(strtok(NULL, blank_chars));
                float g = atof(strtok(NULL, blank_chars));
                float b = atof(strtok(NULL, blank_chars));
                material->Ke = SRE_Vec3_to_rgb(r, g, b);
                break;
            }
            case SRE_SPECULAR_EXP:
            {
                float ns = atof(strtok(NULL, blank_chars));
                material->Ns = ns;
                break;
            }
            default:
                break;
        }
    }
    return SRE_SUCCESS;
}

int SRE_Material_bind(sre_material *mtl, sre_program program)
{
    glUniform1ui(SRE_Get_uniform_location(&program, "ks"), mtl->Ks);
    glUniform1f(SRE_Get_uniform_location(&program, "ns"), mtl->Ns);

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
