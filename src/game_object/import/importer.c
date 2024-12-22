#include <sre/game_object/game_object.h>
#include <sre/game_object/import/importer.h>

static sre_token token_table[64];

const char const *token_strings[] = {
    "endobj",
    "mesh",
    "transform",
    "use_arm",
    "use_mtl",
    "vtx_count",
    "vco",
    "vnormal",
    "vgroup",
    "poly_count",
    "poly",
    "arm",
    "bone_count",
    "act_count",
    "act",
    "kf_count",
    "kf",
    "mtl",
    "diffuse_tex",
    "diffuse",
    "specular",
    "specular_exp",
    "emission",
};

const sre_keyword keywords[] = {
    SRE_ENDOBJ,
    SRE_MESH,
    SRE_TRANSFORM,
    SRE_USE_ARM,
    SRE_USE_MTL,
    SRE_VTX_COUNT,
    SRE_VCO,
    SRE_VNORMAL,
    SRE_VGROUP,
    SRE_POLY_COUNT,
    SRE_POLY,
    SRE_ARM,
    SRE_BONE_COUNT,
    SRE_ACT_COUNT,
    SRE_ACT,
    SRE_KF_COUNT,
    SRE_KF,
    SRE_MTL,
    SRE_DIFFUSE_TEX,
    SRE_DIFFUSE,
    SRE_SPECULAR,
    SRE_SPECULAR_EXP,
    SRE_EMISSION,
};


unsigned char SRE_Get_token_hash(const char *keyword)
{
    char hash = 0;
    char c;
    while (c = *keyword++)
    {
        hash = hash ^ c;

    }

    return hash & 0x3f; // hash % 64
}

int SRE_Add_token(sre_token* token_table, const char *token_string, sre_keyword type)
{
    unsigned char hash = SRE_Get_token_hash(token_string);
    sre_token node = token_table[hash];
    if (node.keyword == SRE_EMPTY)
    {
        node.key = token_string;
        node.index = hash;
        node.collision_idx = 255;
        node.keyword = type;
        token_table[hash] = node;
        return SRE_SUCCESS;
    }
    uint8_t collision_idx = node.collision_idx;
    while (collision_idx != 255)
    {
        hash = collision_idx;
        collision_idx = token_table[hash].collision_idx;
    }

    uint8_t i = 1;
    uint8_t old_hash = hash;
    do {
        hash = (hash + i++) & 0x3f; // hash % 64
    }
    while (token_table[hash].keyword != SRE_EMPTY || hash == old_hash);

    if (hash == old_hash)
    {
        return SRE_ERROR;
    }
    token_table[old_hash].collision_idx = hash;
    token_table[hash].key = token_string;
    token_table[hash].index = hash;
    token_table[hash].collision_idx = -1;
    token_table[hash].keyword = type;

    return SRE_SUCCESS;

}

int SRE_Importer_init(char **token_strings, sre_keyword *keywords, int keywords_count, size_t always_loaded_assets_size, size_t current_zone_assets_size)
{
    for (size_t i = 0; i < 64; i++)
    {
        token_table[i].keyword = SRE_EMPTY;
    }

    for (size_t i = 0; i < keywords_count; i++)
    {
        SRE_Add_token(token_table, token_strings[i], keywords[i]);
    }

    return SRE_SUCCESS;
}

int SRE_Importer_init_default()
{
    size_t always_loaded_assets_size = main_allocator.size / 3;
    return SRE_Importer_init(token_strings, keywords, 23, always_loaded_assets_size, main_allocator.size - always_loaded_assets_size);
}

int SRE_Get_keyword(const char *token_string, sre_keyword *keyword)
{
    uint8_t hash = SRE_Get_token_hash(token_string);
    uint8_t old_hash = hash;
    sre_token node = token_table[hash];
    while (strcmp(node.key, token_string) != 0)
    {
        hash = node.collision_idx;
        if (node.collision_idx == 255)
        {
            *keyword = SRE_UNDEFINED;
            return SRE_SUCCESS;
        }

        node = token_table[hash];
    }

    *keyword = node.keyword;
    return SRE_SUCCESS;
}

int SRE_Armature_process(FILE *file, fpos_t *position, sre_armature *armature)
{
    char buffer[1028];
    int64_t action_idx = -1, kf_idx = -1;
    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, blank_chars);
        sre_keyword keyword;
        SRE_Get_keyword(token, &keyword);
        switch (keyword)
        {
            case SRE_ACT_COUNT:
            {
                armature->action_count = atoi(strtok(NULL, blank_chars));
                int status = SRE_Bump_alloc(&main_allocator, (void**)&armature->actions, armature->action_count * sizeof(sre_action));
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
                strncpy(armature->actions[action_idx].name, name, 64);
                armature->actions[action_idx].bone_count = armature->bone_count;
                break;
            case SRE_KF_COUNT:
            {
                armature->actions[action_idx].keyframe_count = atoi(strtok(NULL, blank_chars));
                int status = SRE_Bump_alloc(&main_allocator, (void**)&armature->actions[action_idx].keyframes, armature->actions[action_idx].keyframe_count * sizeof(sre_keyframe));
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
                int status = SRE_Bump_alloc(&main_allocator, (void**)&armature->actions[action_idx].keyframes[kf_idx].bone_matrices, armature->bone_count * 16 * sizeof(float));
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

int SRE_Material_process(FILE *file, fpos_t *position, sre_material *material)
{
    char buffer[1028];
    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, blank_chars);
        sre_keyword keyword;
        SRE_Get_keyword(token, &keyword);
        switch (keyword)
        {
            case SRE_DIFFUSE_TEX:
            {

                char *texture_name = strtok(NULL, blank_chars);
                sre_game_object *texture_object;
                SRE_Game_object_create(texture_name, &texture_object);
                texture_object->base.go_type = SRE_GO_TEXTURE;
                int status = SRE_Load_texture(texture_name, &texture_object->texture);
                if (status != SRE_SUCCESS)
                {
                    strncpy(texture_object->base.name, texture_name, 64);
                    SRE_Bump_alloc(&main_allocator, &texture_object->texture, sizeof(sre_texture));
                    const char *extension = ".png";
                    char texture_path[256] = "../resources/textures/";
                    strncat(texture_path, texture_name, 256);
                    strncat(texture_path, extension, 256);
                    status = SRE_Load_texture(texture_path, &texture_object->texture);
                    if (status != SRE_SUCCESS)
                    {
                        return status;
                    }
                }

                material->map_Kd = &texture_object->texture;
                break;
            }
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

int SRE_Mesh_process(FILE *file, fpos_t *position, sre_mesh *mesh)
{
    char buffer[1028];
    uint64_t vco_idx = 0, vnormal_idx = 0, vgroup_idx = 0, vidx_idx = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, blank_chars);
        sre_keyword keyword;
        SRE_Get_keyword(token, &keyword);
        sre_game_object *object;
        switch (keyword)
        {
            case SRE_USE_ARM:
            {
                const char* arm_name = strtok(NULL, blank_chars);
                int status = SRE_Game_object_get(arm_name, &object);
                if (status != SRE_SUCCESS)
                {
                    return SRE_ERROR;
                }
                mesh->armature = &object->armature;
                break;
            }
            case SRE_USE_MTL:
            {
                const char* mtl_name = strtok(NULL, blank_chars);
                int status = SRE_Game_object_get(mtl_name, &object);
                if (status != SRE_SUCCESS)
                {
                    return SRE_ERROR;
                }
                mesh->material = &object->material;
                break;
            }
            case SRE_VTX_COUNT:
            {
                mesh->vertex_count = atoi(strtok(NULL, blank_chars));
                SRE_Bump_alloc(&main_allocator, (void**)&mesh->vertex_positions, mesh->vertex_count * sizeof(sre_float_vec3));
                SRE_Bump_alloc(&main_allocator, (void**)&mesh->vertex_normals, mesh->vertex_count * sizeof(sre_2_10_10_10s));
                SRE_Bump_alloc(&main_allocator, (void**)&mesh->bones, mesh->vertex_count * sizeof(ivec4));
                SRE_Bump_alloc(&main_allocator, (void**)&mesh->weights, mesh->vertex_count * sizeof(vec4));
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
                SRE_Bump_alloc(&main_allocator, (void**)&mesh->vertex_indices, mesh->index_count * sizeof(uint64_t));
                SRE_Bump_alloc(&main_allocator, (void**)&mesh->polygon_normals, mesh->index_count * sizeof(sre_2_10_10_10s));
                SRE_Bump_alloc(&main_allocator, (void**)&mesh->uv_coordinates, mesh->index_count * sizeof(sre_norm_16_vec2));
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

int SRE_Import_asset(const char *filepath)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        fprintf(stderr, "Can't open %s\n", filepath);
        return SRE_ERROR;
    }
    uint16_t mesh_array_size = 0;
    uint16_t mesh_idx = 0;
    char directory[256];
    unsigned long long last_slash = strrchr(filepath, '/');
    uint8_t directory_size = ((last_slash - (unsigned long long)filepath) + 1);
    directory_size = directory_size < 256 ? directory_size : 256;
    if (last_slash)
    {
        strncpy(directory, filepath, directory_size);
    }
    char *filename;
    size_t filepath_len = strlen(filepath);
    char buffer[1028];
    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, blank_chars);
        sre_keyword keyword;
        sre_game_object *object;
        fpos_t position;
        SRE_Get_keyword(token, &keyword);
        switch (keyword)
        {
        case SRE_MESH:
        {
            const char *name = strtok(NULL, blank_chars);
            SRE_Game_object_create(name, &object);
            object->base.go_type = SRE_GO_MESH;

            fgetpos(file, &position);
            int status =  SRE_Mesh_process(file, &position, &object->mesh);
            if (status != SRE_SUCCESS)
            {
                return status;
            }
            break;
        }

        case SRE_MTL:
        {
            const char *name = strtok(NULL, blank_chars);
            SRE_Game_object_create(name, &object);
            object->base.go_type = SRE_GO_MATERIAL;

            fgetpos(file, &position);
            int status =  SRE_Material_process(file, &position, &object->material);
            if (status != SRE_SUCCESS)
            {
                return status;
            }
            break;
        }

        case SRE_ARM:
        {
            const char *name = strtok(NULL, blank_chars);
            SRE_Game_object_create(name, &object);
            object->base.go_type = SRE_GO_ARMATURE;

            fpos_t position;
            fgetpos(file, &position);
            int status =  SRE_Armature_process(file, &position, &object->armature);
            if (status != SRE_SUCCESS)
            {
                return status;
            }
            break;
        }

        default:
            break;
        }
    }


    return SRE_SUCCESS;
}

int SRE_Import_collision(const char *filepath)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        fprintf(stderr, "Can't open %s\n", filepath);
        return SRE_ERROR;
    }
    sre_game_object *collider_object;
    char buffer[64];
    fgets(buffer, 64, file);
    const char *name = strtok(buffer, blank_chars);
    SRE_Game_object_create(name, &collider_object);
    collider_object->base.go_type = SRE_GO_COLLIDER;
    sre_collider *collider = &collider_object->collider;
    collider->type = SRE_COLLIDER_BSP_TREE;

    uint16_t size;
    fread(&size, sizeof(uint16_t), 1, file);
    collider->data.bsp_tree.size = size;

    SRE_Bump_alloc(&main_allocator, (void**)&collider->data.bsp_tree.tree, size * sizeof(sre_bsp_node));
    for (size_t i = 0; i < size; i++)
    {
        sre_bsp_node *bsp_tree = &collider->data.bsp_tree.tree[i];
        fread(bsp_tree->deviding_plane, sizeof(float), 4, file);
        fread(&bsp_tree->children[0], sizeof(uint16_t), 1, file);
        fread(&bsp_tree->children[1], sizeof(uint16_t), 1, file);
    }

    return SRE_SUCCESS;
}
