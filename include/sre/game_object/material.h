#ifndef SRE_MATERIAL_H
#define SRE_MATERIAL_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sre/types.h>
#include <sre/hashmap.h>
#include <sre/mem_allocation.h>
#include <sre/game_object/texture.h>
#include <sre/shaders.h>
#include <sre/game_object/game_object_base.h>

#define SRE_MATERIAL_TABLE_SIZE 256

typedef struct struct_sre_material
{
    sre_game_object_base base;
    sre_rgba Kd;
    sre_rgba Ke;
    uint16_t Ns;
    uint16_t Ks;
    sre_texture *map_Kd;
}
sre_material;

int SRE_Material_bind(sre_material *mtl, sre_program *program);

#endif
