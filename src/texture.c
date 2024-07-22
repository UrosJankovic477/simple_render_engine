#include "texture.h"

int SRE_Load_texture(const char *path, sre_texture *texture_out)
{
    glGenTextures(1, &texture_out->id );
    texture_out->data = stbi_load(path, &texture_out->width, &texture_out->height, &texture_out->nChannels, 0);
    if (!texture_out->data)
    {
        printf("%s\n", stbi_failure_reason());
        return SRE_ERROR;
    }
    
    
    glBindTexture(GL_TEXTURE_2D, texture_out->id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_out->width, texture_out->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_out->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return SRE_SUCCESS;
}

int SRE_Delete_texture(sre_texture texture)
{
    glDeleteTextures(1, & texture.id);
    stbi_image_free(texture.data);
    return SRE_SUCCESS;
}
