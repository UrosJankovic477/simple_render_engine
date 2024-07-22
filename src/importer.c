#include "importer.h"

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

int SRE_Importer_init(sre_importer *importer, char **token_strings, sre_keyword *keywords, int keywords_count, size_t always_loaded_assets_size, size_t current_zone_assets_size)
{
    for (size_t i = 0; i < 64; i++)
    {
        importer->token_table[i].keyword = SRE_EMPTY;
    }
    
    for (size_t i = 0; i < keywords_count; i++)
    {
        SRE_Add_token(importer->token_table, token_strings[i], keywords[i]);
    }

    for (size_t i = 0; i < SRE_MAX_OBJECT_COUNT; i++)
    {
        importer->object_table[i].object = NULL;
        importer->object_table[i].empty = true;
        importer->object_table[i].collision = -1;
    }
    
    importer->object_count = 0;

    int status = SRE_Mempool_create(&main_mempool, &importer->always_loaded_assets, always_loaded_assets_size);
    if (status != SRE_SUCCESS)
    {
        return status;
    }

    status = SRE_Mempool_create(&main_mempool, &importer->current_zone_assets, current_zone_assets_size);
    return status;
}

int SRE_Importer_init_default(sre_importer *importer)
{
    size_t always_loaded_assets_size = main_mempool.size / 3;
    return SRE_Importer_init(importer, token_strings, keywords, 23, always_loaded_assets_size, main_mempool.size - always_loaded_assets_size);
}

int SRE_Get_keyword(sre_importer *importer, const char *token_string, sre_keyword *keyword)
{
    uint8_t hash = SRE_Get_token_hash(token_string);
    uint8_t old_hash = hash;
    sre_token node = importer->token_table[hash];
    while (strcmp(node.key, token_string) != 0)
    {
        hash = node.collision_idx;
        if (node.collision_idx == 255)
        {
            *keyword = SRE_UNDEFINED; 
            return SRE_SUCCESS;
        }

        node = importer->token_table[hash];
    }

    *keyword = node.keyword;
    return SRE_SUCCESS;
}

int SRE_Importer_get_object(sre_importer *importer, const char *name, sre_object *object)
{
    uint16_t hash = get_string_hash16(name) & 2047u;
    uint16_t idx = (uint16_t)hash;
    while (strcmp(name, importer->object_table[idx].name) != 0)
    {
        idx = importer->object_table[idx].collision;
        if (idx == 0xffff )
        {
            break;
        }
    }
    if (idx == 0xffff)
    {   
        return SRE_ERROR;
    }
    if (importer->object_table[idx].empty)
    {
        return SRE_ERROR;
    }
    
    *object = importer->object_table[idx];

    return SRE_SUCCESS;
}

int SRE_Importer_add_object(sre_importer *importer, sre_object object)
{
    if (importer->object_count == SRE_MAX_OBJECT_COUNT)
    {
        return SRE_ERROR;
    }
    uint16_t hash = get_string_hash16(object.name) & 2047u;
    sre_object node = importer->object_table[hash];
    if (node.empty)
    {
        strcpy_s(importer->object_table[hash].name, 64, object.name);
        importer->object_table[hash].index = hash;
        importer->object_table[hash].object = object.object;
        importer->object_table[hash].empty = false;
        return SRE_SUCCESS;
    }
    uint8_t i = 1;
    uint8_t old_hash = hash;
    do {
        hash = (hash + i++) & 0xff; // hash % 256
    }
    while (importer->object_table[hash].empty == false || hash == old_hash);
    
    if (hash == old_hash)
    {
        return SRE_ERROR;
    }
    importer->object_table[old_hash].collision = hash;
    strcpy_s(importer->object_table[hash].name, 64, object.name);
    importer->object_table[hash].index = hash;
    importer->object_table[hash].object = object.object;
    importer->object_table[hash].empty = false;

    return SRE_SUCCESS;
}

int SRE_Importer_remove_object(sre_importer *importer, const char * name)
{
    uint16_t hash = get_string_hash16(name) & 2047u;
    uint16_t idx = (uint16_t)hash;
    while (strcmp(name, importer->object_table[idx].name) != 0 && idx != 0xffff)
    {
        idx = importer->object_table[idx].collision;
    }
    if (idx == 0xffff)
    {   
        return SRE_ERROR;
    }
    importer->object_table[idx].empty == true;

    return SRE_SUCCESS;
}
