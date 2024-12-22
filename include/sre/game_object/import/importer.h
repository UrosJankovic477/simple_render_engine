#ifndef SRE_IMPORTER_H
#define SRE_IMPORTER_H

#include <string.h>
#include <stdbool.h>
#include <sre/mem_allocation.h>
#include <sre/logging/errors.h>
#include <sre/hashmap.h>

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

typedef struct struct_sre_token
{
    sre_keyword keyword;
    const char *key;
    uint8_t collision_idx;
    uint8_t index;
}
sre_token;

int SRE_Importer_init(char **keywords, sre_keyword *keyword_types, int keywords_count, size_t always_loaded_assets_size, size_t current_zone_assets_size);
int SRE_Importer_init_default();
int SRE_Get_keyword(const char *token_string, sre_keyword *keyword);
int SRE_Import_asset(const char *filepath);
int SRE_Import_collision(const char *filepath);


#endif
