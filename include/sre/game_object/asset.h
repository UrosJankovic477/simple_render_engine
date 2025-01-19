#ifndef SRE_ASSET_H
#define SRE_ASSET_H
#include <sre/game_object/group.h>
#include <sre/game_object/mesh.h>

typedef union union_sre_asset
{
    struct 
    {
        sre_game_object_base base;
        uint32_t loaded_instance_count;
    };

    sre_group group;
    sre_mesh mesh;
}
sre_asset;

typedef struct struct_sre_asset_instance
{
    sre_game_object_base base;
    sre_transform transform;
    sre_asset *asset;
    bool loaded
}
sre_asset_instance;

void SRE_Asset_create_instance(sre_asset *asset, sre_transform transform, const char *name, sre_asset_instance **instance);

void SRE_Asset_draw_insance(sre_asset_instance *instance, sre_program *program, uint32_t types);

void SRE_Asset_instance_make_real(sre_asset_instance *instance, sre_asset *object);

void SRE_Asset_instance_load(sre_asset_instance *instance);

void SRE_Asset_instance_unload(sre_asset_instance *instance);

#endif