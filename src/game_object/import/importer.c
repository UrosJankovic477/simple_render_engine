#include <sre/game_object/game_object.h>
#include <sre/game_object/import/importer.h>
#include <SDL2/SDL_endian.h>

typedef enum enum_sre_keyword
{
    SRE_EMPTY,
    SRE_UNDEFINED,
    SRE_MESH,
    SRE_USE_ARM,
    SRE_USE_MTL,
    SRE_ARM,
    SRE_MTL,
    SRE_DIFFUSE_TEX,
    SRE_DIFFUSE,
}
sre_keyword;

typedef struct struct_sre_token
{
    sre_keyword keyword;
    const char *key;
    uint8_t collision_idx;
    uint8_t index;
}
sre_token;

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
    SRE_MESH,
    SRE_USE_ARM,
    SRE_USE_MTL,
    SRE_ARM,
    SRE_MTL,
    SRE_DIFFUSE_TEX,
    SRE_DIFFUSE,
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
    uint16_t bone_count, action_count;
    fread(&bone_count, sizeof(uint16_t), 1, file);
    fread(&action_count, sizeof(uint16_t), 1, file);
    armature->action_count = SDL_SwapLE16(action_count);
    int status = SRE_Bump_alloc(&main_allocator, (void**)&armature->actions, armature->action_count * sizeof(sre_action));
    if (status != SRE_SUCCESS)
    {
        return SRE_ERROR;
    }

    armature->bone_count = SDL_SwapLE16(bone_count);

    for (uint16_t action_idx = 0; action_idx < action_count; action_idx++)
    {
        char buffer[64];
        fgets(buffer, 64, file);
        const char *name = strtok(buffer, blank_chars);
        strncpy(armature->actions[action_idx].name, name, 64);
        armature->actions[action_idx].bone_count = armature->bone_count;

        uint16_t keyframe_count;
        fread(&keyframe_count, sizeof(uint16_t), 1, file);
        armature->actions[action_idx].keyframe_count = SDL_SwapLE16(keyframe_count);
        int status = SRE_Bump_alloc(&main_allocator, (void**)&armature->actions[action_idx].keyframes, armature->actions[action_idx].keyframe_count * sizeof(sre_keyframe));
        if (status != SRE_SUCCESS)
        {
            return SRE_ERROR;
        }
        for (uint16_t kf_idx = 0; kf_idx < keyframe_count; kf_idx++)
        {
            uint32_t timestamp;
            fread(&timestamp, sizeof(uint32_t), 1, file);
            armature->actions[action_idx].keyframes[kf_idx].timestamp = SDL_SwapLE16(timestamp);
            int status = SRE_Bump_alloc(&main_allocator, (void**)&armature->actions[action_idx].keyframes[kf_idx].bone_matrices, armature->bone_count * 16 * sizeof(float));
            if (status != SRE_SUCCESS)
            {
                return SRE_ERROR;
            }
            for (size_t bone_idx = 0; bone_idx < armature->bone_count; bone_idx++)
            {
                for (size_t i = 0; i < 4; i++)
                {
                    for (size_t j = 0; j < 4; j++)
                    {
                        float mat_element;
                        fread(&mat_element, sizeof(float), 1, file);
                        armature->actions[action_idx].keyframes[kf_idx].bone_matrices[bone_idx][i][j] = SDL_SwapLE16(mat_element);
                    }
                }
            }
        }
    }

    return SRE_SUCCESS;
}

int SRE_Material_process(FILE *file, fpos_t *position, sre_material *material)
{
    uint8_t keyword;
    fread(&keyword, sizeof(uint8_t), 1, file);

    //Diffuse
    switch (keyword)
    {
        case SRE_DIFFUSE_TEX:
        {
            char buffer[256];
            fgets(buffer, 256, file);
            char *texture_name = strtok(buffer, blank_chars);
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
            uint8_t rgba[4];
            fread(rgba, sizeof(uint8_t), 4, file);
            material->Kd.r = rgba[0];
            material->Kd.g = rgba[1];
            material->Kd.b = rgba[2];
            material->Kd.a = rgba[3];
            break;
        }
        default:
            return SRE_ERROR;
    }

    //Specular
    uint16_t specular[2];
    fread(specular, sizeof(uint16_t), 2, file);
    material->Ks = SDL_SwapLE16(specular[0]);
    material->Ns = SDL_SwapLE16(specular[1]);

    //Emission
    uint8_t em_rgb[3];
    fread(em_rgb, sizeof(uint8_t), 3, file);
    material->Ke.r = em_rgb[0];
    material->Ke.g = em_rgb[1];
    material->Ke.b = em_rgb[2];
    material->Ke.a = 255;

    return SRE_SUCCESS;
}

int SRE_Mesh_process(FILE *file, fpos_t *position, sre_mesh *mesh)
{
    uint64_t vco_idx = 0, vnormal_idx = 0, vgroup_idx = 0, vidx_idx = 0;
    // Transform
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            float mat_element;
            fread(&mat_element, sizeof(float), 1, file);
            mesh->model_mat[i][j] = SDL_SwapFloatLE(mat_element);
        }
    }
    uint8_t keyword;
    fread(&keyword, sizeof(uint8_t), 1, file);

    // Armature
    char buffer[64];
    if (keyword == SRE_USE_ARM)
    {
        sre_game_object *armature;
        fgets(buffer, 64, file);
        const char* arm_name = strtok(buffer, blank_chars);
        int status = SRE_Game_object_get(arm_name, &armature);
        strncpy(mesh->armature_name, arm_name, 64);
        mesh->armature = &armature->armature;
        fread(&keyword, sizeof(uint8_t), 1, file);
    }
    else
    {
        mesh->armature = NULL;
        mesh->armature_name[0] = '\0';
    }


    // Material
    if (keyword == SRE_USE_MTL)
    {
        sre_game_object *material;
        fgets(buffer, 64, file);
        const char* mtl_name = strtok(buffer, blank_chars);
        int status = SRE_Game_object_get(mtl_name, &material);
        strncpy(mesh->material_name, mtl_name, 64);
        mesh->material = &material->material;
    }

    // Get number of vertices
    uint32_t vertex_count;
    fread(&vertex_count, sizeof(uint32_t), 1, file);
    mesh->vertex_count = SDL_SwapLE16(vertex_count);
    SRE_Bump_alloc(&main_allocator, (void**)&mesh->vertex_positions, mesh->vertex_count * sizeof(sre_float_vec3));
    SRE_Bump_alloc(&main_allocator, (void**)&mesh->vertex_normals, mesh->vertex_count * sizeof(sre_2_10_10_10s));
    SRE_Bump_alloc(&main_allocator, (void**)&mesh->bones, mesh->vertex_count * sizeof(ivec4));
    SRE_Bump_alloc(&main_allocator, (void**)&mesh->weights, mesh->vertex_count * sizeof(vec4));

    // Vertices
    for (uint32_t i = 0; i < vertex_count; i++)
    {
        float xyz[3];
        fread(xyz, sizeof(float), 3, file);
        mesh->vertex_positions[i][0] = SDL_SwapFloatLE(xyz[0]);
        mesh->vertex_positions[i][1] = SDL_SwapFloatLE(xyz[1]);
        mesh->vertex_positions[i][2] = SDL_SwapFloatLE(xyz[2]);
        sre_2_10_10_10s normal;
        fread(&normal, sizeof(sre_2_10_10_10s), 1, file);
        mesh->vertex_normals[i] = SDL_SwapLE32(normal);
        uint8_t vgroup_count;
        fread(&vgroup_count, sizeof(uint8_t), 1, file);
        glm_vec4_zero(mesh->weights[i]);
        glm_ivec4_zero(mesh->bones[i]);
        for (uint8_t j = 0; j < vgroup_count; j++)
        {
            uint32_t group;
            float weight;
            fread(&group, sizeof(uint32_t), 1, file);
            fread(&weight, sizeof(float), 1, file);
            mesh->bones[i][j] = SDL_SwapLE32(group);
            mesh->weights[i][j] = SDL_SwapFloatLE(weight);
        }
    }

    // Polygons
    uint32_t poly_count_count;
    fread(&poly_count_count, sizeof(uint32_t), 1, file);

    uint32_t index_count = SDL_SwapLE32(poly_count_count) * 3;
    mesh->index_count = index_count;
    SRE_Bump_alloc(&main_allocator, (void**)&mesh->vertex_indices, mesh->index_count * sizeof(uint32_t));
    SRE_Bump_alloc(&main_allocator, (void**)&mesh->polygon_normals, mesh->index_count * sizeof(sre_2_10_10_10s));
    SRE_Bump_alloc(&main_allocator, (void**)&mesh->uv_coordinates, mesh->index_count * sizeof(sre_norm_16_vec2));

    for (size_t i = 0; i < index_count; i += 3)
    {
        for (size_t j = 0; j < 3; j++)
        {
            uint32_t vertex_index;
            uint16_t uv[2];
            fread(&vertex_index, sizeof(uint32_t), 1, file);
            fread(uv, sizeof(uint16_t), 2, file);
            mesh->vertex_indices[i + j] = SDL_SwapLE32(vertex_index);
            mesh->uv_coordinates[i + j].x = SDL_SwapLE16(uv[0]);
            mesh->uv_coordinates[i + j].y = SDL_SwapLE16(uv[1]);
        }
        sre_2_10_10_10s poly_normal;
        fread(&poly_normal, sizeof(sre_2_10_10_10s), 1, file);
        mesh->polygon_normals[i] = SDL_SwapLE32(poly_normal);
        mesh->polygon_normals[i + 1] = mesh->polygon_normals[i];
        mesh->polygon_normals[i + 2] = mesh->polygon_normals[i];
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
    uint8_t keyword;
    while (fread(&keyword, sizeof(uint8_t), 1, file))
    {
        sre_game_object *object;
        fpos_t position;
        switch (keyword)
        {
        case SRE_MESH:
        {
            char buffer[64];
            fgets(buffer, 64, file);
            const char *name = strtok(buffer, blank_chars);
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
            char buffer[64];
            fgets(buffer, 64, file);
            const char *name = strtok(buffer, blank_chars);
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
            char buffer[64];
            fgets(buffer, 64, file);
            const char *name = strtok(buffer, blank_chars);
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
