#include <sre/game_object/mesh.h>
#include <sre/game_object/group.h>
#include <sre/game_object/game_object.h>
#include <cglm/call/affine.h>
#include <cglm/call/mat4.h>

uint8_t mesh_count = 0;

void SRE_Mesh_transform(sre_mesh *mesh, mat4 transform)
{
    glm_mat4_mul(transform, mesh->model_mat, mesh->model_mat);
}

int SRE_Mesh_load(sre_mesh *mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);
    glGenBuffers(1, &mesh->vbo);

    if (mesh->material == NULL)
    {
        SRE_Game_object_get(mesh->material_name, &mesh->material);
    }
    if (mesh->armature == NULL && mesh->armature_name[0] != '\0')
    {
        SRE_Game_object_get(mesh->armature_name, &mesh->armature);
    }

    sre_vertex vertices[MAX_VERTEX_COUNT];
    for (size_t i = 0; i < mesh->index_count; i++)
    {
        uint64_t vertex_idx = mesh->vertex_indices[i];
        vertices[i].posx = mesh->vertex_positions[vertex_idx][0];
        vertices[i].posy = mesh->vertex_positions[vertex_idx][1];
        vertices[i].posz = mesh->vertex_positions[vertex_idx][2];
        vertices[i].normal = mesh->vertex_normals[vertex_idx];
        vertices[i].u = mesh->uv_coordinates[i].x;
        vertices[i].v = mesh->uv_coordinates[i].y;
        vertices[i].bone1 = (uint32_t)mesh->bones[vertex_idx][0];
        vertices[i].bone2 = (uint32_t)mesh->bones[vertex_idx][1];
        vertices[i].bone3 = (uint32_t)mesh->bones[vertex_idx][2];
        vertices[i].bone4 = (uint32_t)mesh->bones[vertex_idx][3];
        vertices[i].w1 = mesh->weights[vertex_idx][0];
        vertices[i].w2 = mesh->weights[vertex_idx][1];
        vertices[i].w3 = mesh->weights[vertex_idx][2];
        vertices[i].w4 = mesh->weights[vertex_idx][3];
    }

    size_t stride = sizeof(sre_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(sre_vertex) * mesh->index_count, (void*)vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    GLvoid *pointer = NULL;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, pointer);
    pointer += 3 * sizeof(float);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride, pointer);
    pointer += 2 * sizeof(sre_norm_16);
    glVertexAttribPointer(2, 4, GL_INT_2_10_10_10_REV, GL_TRUE, stride, pointer);
    pointer += sizeof(sre_2_10_10_10s);
    glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, stride, pointer);
    pointer += 4 * sizeof(int);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, pointer);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return SRE_SUCCESS;
}

int SRE_Mesh_unload(sre_mesh *mesh)
{
    glDeleteBuffers(1, &(mesh->vbo));
    glDeleteVertexArrays(1, &(mesh->vao));
    return 0;
}

int SRE_Mesh_draw(sre_mesh *mesh, sre_program program)
{
    sre_material *mtl = mesh->material;
    sre_armature *arm = mesh->armature;


    if (arm)
    {
        glUniform1ui(SRE_Get_uniform_location(&program, "animated"), true);
    }
    else
    {
        glUniform1ui(SRE_Get_uniform_location(&program, "animated"), false);
    }

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    mat4 model;
    glmc_mat4_mul(mesh->base.parent_transform_mat, mesh->model_mat, model);

    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "model"), 1, GL_FALSE, model);
    SRE_Material_bind(mtl, program);

    glDrawArrays(GL_TRIANGLES, 0, mesh->index_count);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return SRE_SUCCESS;
}
