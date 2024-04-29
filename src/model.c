#include "model.h"

int SRE_Load_gl_mesh(sre_mesh_obj *p_mesh, gl_mesh *p_gl_mesh)
{
    glGenVertexArrays(1, &p_gl_mesh->vao);
    glBindVertexArray(p_gl_mesh->vao);
    glGenBuffers(1, &p_gl_mesh->vbo);
    
    sre_vertex *mesh_data;
    size_t size = sizeof(sre_vertex) * p_mesh->indecies_count;
    unsigned int stride = sizeof(sre_vertex);
    mesh_data = malloc(size);

    p_gl_mesh->vertex_count = p_mesh->indecies_count;

    for (size_t i = 0; i < p_mesh->indecies_count; i++)
    {
        unsigned int pos_idx = p_mesh->indecies[0][i] - 1;
        unsigned int uv_idx = p_mesh->indecies[1][i] - 1;
        unsigned int normal_idx = p_mesh->indecies[2][i] - 1;

        mesh_data[i].posx = p_mesh->position[pos_idx].x;
        mesh_data[i].posy = p_mesh->position[pos_idx].y;
        mesh_data[i].posz = p_mesh->position[pos_idx].z;
        mesh_data[i].normal = p_mesh->normal[normal_idx];
        mesh_data[i].u = p_mesh->uv[0][uv_idx].x;
        mesh_data[i].v = p_mesh->uv[0][uv_idx].y;
    }
    p_gl_mesh->material_count = p_mesh->group_count;
    p_gl_mesh->materials = malloc(p_mesh->group_count * sizeof(sre_material*));
    p_gl_mesh->group_indecies = malloc(p_mesh->group_count * sizeof(int));
    for (size_t i = 0; i < p_mesh->group_count; i++)
    {
        sre_group_obj group = p_mesh->groups[i];
        if (group.material)
        {
            p_gl_mesh->materials[i] = group.material;
        }
        else
        {
            p_gl_mesh->materials[i] = get_material(group.material_name);
        }
        p_gl_mesh->group_indecies[i] = group.index;
    }
    

    glBindBuffer(GL_ARRAY_BUFFER, p_gl_mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, size, mesh_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride, (GLvoid*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 4, GL_INT_2_10_10_10_REV, GL_TRUE, stride, (GLvoid*)(3 * sizeof(float) + 2 * sizeof(sre_norm_16)));
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    

    return EXIT_SUCCESS;
}

int SRE_Draw_gl_mesh(gl_mesh *p_gl_mesh, sre_program program, mat4 model)
{
    glBindVertexArray(p_gl_mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, p_gl_mesh->vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    unsigned int start_idx = 0;

    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "model"), 1, GL_FALSE, model);

    for (size_t i = 0; i < p_gl_mesh->material_count; i++)
    {
        sre_material *mtl = p_gl_mesh->materials[i];

        glUniform1ui(SRE_Get_uniform_location(&program, "ka"), mtl->Ka.rgba);
        glUniform1ui(SRE_Get_uniform_location(&program, "kd"), mtl->Kd.rgba);
        glUniform1ui(SRE_Get_uniform_location(&program, "ks"), mtl->Ks.rgba);
        glUniform1f(SRE_Get_uniform_location(&program, "ns"), mtl->Ns);

        //glUniform1i(get_uniform_location(&program, "diffuse_tex_sampler"), 0);

        if (p_gl_mesh->materials[i]->map_Kd)
        {
            glBindTexture(GL_TEXTURE_2D, p_gl_mesh->materials[i]->map_Kd->id);
            glUniform1ui(SRE_Get_uniform_location(&program, "use_diffuse_tex"), GL_TRUE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, 0);
            glUniform1ui(SRE_Get_uniform_location(&program, "use_diffuse_tex"), GL_FALSE);
        }
        

        unsigned int end_idx = p_gl_mesh->group_indecies[i] > p_gl_mesh->vertex_count ? 
            p_gl_mesh->vertex_count : p_gl_mesh->group_indecies[i];
        unsigned int count = end_idx - start_idx;
        glDrawArrays(GL_TRIANGLES, start_idx, count);

        start_idx = end_idx;
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SRE_Delete_gl_mesh(gl_mesh *mesh)
{
    free(mesh->group_indecies);
    free(mesh->materials);
    glDeleteBuffers(1, &(mesh->vbo));
    glDeleteVertexArrays(1, &(mesh->vao));
}
