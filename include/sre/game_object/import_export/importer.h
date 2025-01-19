#ifndef SRE_IMPORTER_H
#define SRE_IMPORTER_H

#include <string.h>
#include <stdbool.h>
#include <sre/mem_allocation.h>
#include <sre/logging/errors.h>
#include <sre/hashmap.h>

#define blank_chars " \r\n\t"

typedef enum enum_sre_keyword
{
    SRE_KW_EMPTY,
    SRE_KW_UNDEFINED,
    SRE_KW_MESH,
    SRE_KW_USE_ARM,
    SRE_KW_USE_MTL,
    SRE_KW_ARM,
    SRE_KW_MTL,
    SRE_KW_DIFFUSE_TEX,
    SRE_KW_DIFFUSE,
    SRE_KW_CAMERA,
    SRE_KW_COLLIDER,
    SRE_KW_LIGHT,
    SRE_KW_GROUP,
    SRE_KW_ASSET_INSTANCE,
    SRE_KW_SCENE,
}
sre_keyword;

int SRE_Import_asset(const char *filepath);


#endif
