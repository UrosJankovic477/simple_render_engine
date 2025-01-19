#include <sre/game_object/asset.h>
#include <sre/game_object/game_object.h>
#include <sre/game_object/import_export/importer.h>

void SRE_Asset_create_instance(sre_asset *asset, sre_transform transform, const char *name, sre_asset_instance **instance)
{
    if (name)
    {
        SRE_Game_object_create(name, SRE_GO_ASSET_INSANCE, instance);
    }
    else
    {
        char instance_name[64];
        strncpy(instance_name, asset->base.name, 60);
        SRE_Game_object_create(instance_name, SRE_GO_ASSET_INSANCE, instance);
    }
    
    (*instance)->asset = asset;
    (*instance)->loaded = false;
    SRE_Trans_copy(transform, &((*instance)->transform));
}

void SRE_Asset_draw_insance(sre_asset_instance *instance, sre_program *program, uint32_t types)
{
    sre_asset *asset = instance->asset;
    SRE_Trans_create_mat(&instance->transform, asset->base.parent_transform_mat);
    glmc_mat4_mul(instance->base.parent_transform_mat, asset->base.parent_transform_mat, asset->base.parent_transform_mat);
    if (instance->asset->base.type == SRE_GO_GROUP)
    {
        SRE_Group_draw((sre_group*)instance->asset, program, types);
    }
    else
    {
        SRE_Mesh_draw((sre_mesh*)instance->asset, program);
    }
}

void SRE_Asset_instance_load(sre_asset_instance *instance)
{
    if (instance->loaded)
    {
        return;
    }

    instance->loaded = true;

    if (instance->asset->loaded_instance_count++ > 0)
    {
        return;
    }
    
    if (instance->asset->base.type == SRE_GO_MESH)
    {
        SRE_Mesh_load((sre_mesh*)instance->asset);
    }
    else
    {
        SRE_Group_load((sre_group*)instance->asset, ~0);
    }
    
}

void SRE_Asset_instance_unload(sre_asset_instance *instance)
{
    if (!instance->loaded)
    {
        return;
    }

    instance->loaded = false;
    instance->asset->loaded_instance_count--;
    if (instance->asset->loaded_instance_count == 0)
    {
        if (instance->asset->base.type == SRE_GO_MESH)
        {
            SRE_Mesh_unload((sre_mesh*)instance->asset);
        }
        else
        {
            SRE_Group_unload((sre_group*)instance->asset, ~0);
        }
    
    }
    
    
}
