#include "mesh.h"

uint8_t mesh_count = 0;


int SRE_Mesh_process(sre_importer *importer, sre_mempool *asset_mempool, FILE *file, fpos_t *position, sre_mesh *mesh)
{
    char buffer[1028];
    uint64_t vco_idx = 0, vnormal_idx = 0, vgroup_idx = 0, vidx_idx = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, blank_chars);
        sre_keyword keyword;
        SRE_Get_keyword(importer, token, &keyword);
        switch (keyword)
        {
            case SRE_USE_ARM:
            {
                const char* arm_name = strtok(NULL, blank_chars);
                sre_object object;
                int status = SRE_Importer_get_object(importer, arm_name, &object);
                if (status != SRE_SUCCESS)
                {
                    return SRE_ERROR;
                }
                mesh->armature = (sre_armature*)object.object;
                break;
            }
            case SRE_USE_MTL:
            {
                const char* mtl_name = strtok(NULL, blank_chars);
                sre_object object;
                int status = SRE_Importer_get_object(importer, mtl_name, &object);
                if (status != SRE_SUCCESS)
                {
                    return SRE_ERROR;
                }
                mesh->material = (sre_material*)object.object;
                break;
            }
            case SRE_VTX_COUNT:
            {
                mesh->vertex_count = atoi(strtok(NULL, blank_chars));
                SRE_Mempool_alloc(asset_mempool, (void**)&mesh->vertex_positions, mesh->vertex_count * sizeof(sre_float_vec3));
                SRE_Mempool_alloc(asset_mempool, (void**)&mesh->vertex_normals, mesh->vertex_count * sizeof(sre_2_10_10_10s));
                SRE_Mempool_alloc(asset_mempool, (void**)&mesh->bones, mesh->vertex_count * sizeof(ivec4));
                SRE_Mempool_alloc(asset_mempool, (void**)&mesh->weights, mesh->vertex_count * sizeof(vec4));
                break;
            }
            case SRE_VCO:
            {
                mesh->vertex_positions[vco_idx][0] = atof(strtok(NULL, blank_chars));
                mesh->vertex_positions[vco_idx][1] = atof(strtok(NULL, blank_chars));
                mesh->vertex_positions[vco_idx][2] = atof(strtok(NULL, blank_chars));
                vco_idx++;
                break;
            }
            case SRE_VNORMAL:
            {
                float nx = atof(strtok(NULL, blank_chars));
                float ny = atof(strtok(NULL, blank_chars));
                float nz = atof(strtok(NULL, blank_chars));
                mesh->vertex_normals[vnormal_idx] = SRE_Float_to_2_10_10_10s(nx, ny, nz);
                vnormal_idx++;
                break;
            }
            case SRE_VGROUP:
            {
                uint32_t vgroup_count = atoi(strtok(NULL, blank_chars));
                glm_vec4_zero(mesh->weights[vgroup_idx]);
                glm_ivec4_zero(mesh->bones[vgroup_idx]);
                for (size_t i = 0; i < vgroup_count; i++)
                {
                    mesh->bones[vgroup_idx][i] = atoi(strtok(NULL, blank_chars));
                    mesh->weights[vgroup_idx][i] = atof(strtok(NULL, blank_chars));
                }
                vgroup_idx++;
                break;
            }
            case SRE_POLY_COUNT:
            {
                uint64_t poly_count_count = atoi(strtok(NULL, blank_chars));
                mesh->index_count = poly_count_count * 3;
                SRE_Mempool_alloc(asset_mempool, (void**)&mesh->vertex_indices, mesh->index_count * sizeof(uint64_t));
                SRE_Mempool_alloc(asset_mempool, (void**)&mesh->polygon_normals, mesh->index_count * sizeof(sre_2_10_10_10s));
                SRE_Mempool_alloc(asset_mempool, (void**)&mesh->uv_coordinates, mesh->index_count * sizeof(sre_norm_16_vec2));
                break;
            }
            case SRE_POLY:
            {
                for (size_t i = 0; i < 3; i++)
                {
                    mesh->vertex_indices[vidx_idx + i] = atoi(strtok(NULL, blank_chars));
                    mesh->uv_coordinates[vidx_idx + i].x = SRE_Float_to_unorm_16(atof(strtok(NULL, " ")));
                    mesh->uv_coordinates[vidx_idx + i].y = SRE_Float_to_unorm_16(atof(strtok(NULL, " ")));
                }
                float x = atof(strtok(NULL, blank_chars));
                float y = atof(strtok(NULL, blank_chars));
                float z = atof(strtok(NULL, blank_chars));
                mesh->polygon_normals[vidx_idx] = SRE_Float_to_2_10_10_10s(x, y, z);
                mesh->polygon_normals[vidx_idx + 1] = mesh->polygon_normals[vidx_idx];
                mesh->polygon_normals[vidx_idx + 2] = mesh->polygon_normals[vidx_idx];
                vidx_idx += 3;
                break; 
            }
            case SRE_TRANSFORM:
            {
                for (size_t i = 0; i < 4; i++)
                {
                    for (size_t j = 0; j < 4; j++)
                    {
                        mesh->model_mat[i][j] = atof(strtok(NULL, blank_chars));
                    }
                }
                break;
            }
            default:
                break;
        }
        
    }
    return SRE_SUCCESS;
}

int SRE_Mesh_load(sre_mesh *mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);
    glGenBuffers(1, &mesh->vbo);

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

    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "model"), 1, GL_FALSE, mesh->model_mat);
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
