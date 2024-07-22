#ifndef SRE_MATERIAL_H
#define SRE_MATERIAL_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "types.h"
#include "texture.h"
#include "importer.h"
#include "hashmap.h"
#include "shaders.h"
#define SRE_MATERIAL_TABLE_SIZE 256

typedef struct struct_sre_material {
    char *name;
    uint8_t index;
    uint8_t collision;
    sre_rgba Kd;
    uint8_t Ks;
    sre_rgba Ke;
    float Ns;
    sre_texture *map_Kd;
} sre_material;

int SRE_Material_process(sre_importer *importer, sre_mempool *asset_mempool, FILE *file, fpos_t *position, sre_material *material);
int SRE_Material_bind(sre_material *mtl, sre_program program);

#endif 