#ifndef SRE_IMPORTER_H
#define SRE_IMPORTER_H

#include <string.h>
#include <stdbool.h>
#include "mem_allocation.h"
#include "errors.h"
#include "hashmap.h"

#define SRE_MAX_OBJECT_COUNT 2048
#define blank_chars " \r\n\t"

typedef enum enum_sre_keyword {
    SRE_EMPTY,
    SRE_UNDEFINED,
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
} sre_keyword;

typedef struct struct_sre_object {
    void *object;
    sre_mempool *mempool;
    uint16_t collision;
    uint16_t index;
    char name[64];
    bool empty;
} sre_object;

typedef struct struct_sre_token {
    sre_keyword keyword;
    const char *key;
    uint8_t collision_idx;
    uint8_t index;
} sre_token;

typedef struct struct_sre_importer {
    sre_token token_table[64];
    sre_object object_table[SRE_MAX_OBJECT_COUNT];
    sre_mempool always_loaded_assets; 
    sre_mempool current_zone_assets;
    uint16_t object_count;
} sre_importer;


int SRE_Importer_init(sre_importer *importer, char **keywords, sre_keyword *keyword_types, int keywords_count, size_t always_loaded_assets_size, size_t current_zone_assets_size);
int SRE_Importer_init_default(sre_importer *importer);
int SRE_Get_keyword(sre_importer *importer, const char *token_string, sre_keyword *keyword);
int SRE_Importer_get_object(sre_importer *importer, const char *name, sre_object* object);
int SRE_Importer_add_object(sre_importer *importer, sre_object object);
int SRE_Importer_remove_object(sre_importer *importer, const char *name);

#endif