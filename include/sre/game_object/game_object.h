#ifndef SRE_GAME_OBJECT_H
#define SRE_GAME_OBJECT_H

#include <stdint.h>
#include <stdbool.h>
#include <sre/game_object/game_object_base.h>
#include <sre/game_object/armature.h>
#include <sre/game_object/mesh.h>
#include <sre/game_object/material.h>
#include <sre/game_object/collision.h>
#include <sre/game_object/cntl.h>
#include <sre/game_object/group.h>
#include <sre/game_object/camera.h>
#include <sre/game_object/asset.h>
#include <sre/mem_allocation.h>

#define SRE_MAX_OBJECT_COUNT 2048

typedef union union_sre_game_object
{
    sre_game_object_base base;
    void *null_go;
    sre_mesh mesh;
    sre_armature armature;
    sre_material material;
    sre_texture texture;
    sre_collider collider;
    sre_group group;
    sre_camera camera;
    sre_light light;
    sre_asset_instance asset_insance;
}
sre_game_object;

int SRE_Object_queue_enqueue();

int SRE_Game_object_manager_init();
int SRE_Game_object_get(const char *name, sre_game_object **object);
int SRE_Game_object_create(const char *name, sre_object_type type, sre_game_object **out_object_ptr);
int SRE_Game_object_remove(const char *name);
void SRE_Game_object_remove_id(uint16_t obj_id);
int SRE_Game_object_get_id(uint16_t obj_id, sre_game_object **object);
void SRE_Game_object_get_or_import(const char *name, const char *path, sre_game_object **object);

#endif //SRE_GAME_OBJECT_H
