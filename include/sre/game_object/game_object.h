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
    sre_control_listener control_listener;
    sre_camera camera;
    sre_transform transform;
    sre_light light;
}
sre_game_object;

int SRE_Object_queue_enqueue();

int SRE_Game_object_manager_init();
int SRE_Game_object_get(const char *name, sre_game_object **object);
int SRE_Game_object_create(const char *name, sre_game_object **out_object_ptr);
int SRE_Game_object_remove(const char *name);
int SRE_Get_game_object_from_id(uint16_t obj_id, sre_game_object **object);

#endif //SRE_GAME_OBJECT_H
