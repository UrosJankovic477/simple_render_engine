#include "asset_import.h"

int SRE_Import_asset(sre_importer *importer, const char *filepath, bool always_loaded)
{
    sre_mempool *asset_mempool = always_loaded ? &importer->always_loaded_assets : &importer->current_zone_assets;
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
    if (last_slash)
    {
        strncpy_s(directory, 256, filepath, (last_slash - (unsigned long long)filepath) + 1);
    }
    char *filename;
    size_t filepath_len = strlen(filepath);
    char buffer[1028];
    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, blank_chars);
        sre_keyword keyword;
        sre_object object;
        fpos_t position;
        SRE_Get_keyword(importer, token, &keyword);
        switch (keyword)
        {
        case SRE_MESH:
        {
            const char *name = strtok(NULL, blank_chars);
            strcpy_s(object.name, 64, name);
            SRE_Mempool_alloc(asset_mempool, &object.object, sizeof(sre_mesh));
            
            SRE_Importer_add_object(importer, object);
            
            fgetpos(file, &position);
            int status =  SRE_Mesh_process(importer, asset_mempool, file, &position, object.object);
            if (status != SRE_SUCCESS)
            {
                return status;
            }
            break;
        }

        case SRE_MTL:
        {
            const char *name = strtok(NULL, blank_chars);
            strcpy_s(object.name, 64, name);
            SRE_Mempool_alloc(asset_mempool, &object.object, sizeof(sre_material));
            SRE_Importer_add_object(importer, object);
            fgetpos(file, &position);
            int status =  SRE_Material_process(importer, asset_mempool, file, &position, object.object);
            if (status != SRE_SUCCESS)
            {
                return status;
            }
            break;
        }
        
        case SRE_ARM:
        {

            sre_object object;
            const char *name = strtok(NULL, blank_chars);
            strcpy_s(object.name, 64, name);
            SRE_Mempool_alloc(asset_mempool, &object.object, sizeof(sre_armature));
            SRE_Importer_add_object(importer, object);
            fpos_t position;
            fgetpos(file, &position);
            int status =  SRE_Armature_process(importer, asset_mempool, file, &position, object.object);
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

