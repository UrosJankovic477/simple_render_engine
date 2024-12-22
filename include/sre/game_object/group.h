#ifndef SRE_GROUP_H
#define SRE_GROUP_H

#include <stdint.h>
#include <sre/game_object/collision.h>
#include <sre/game_object/cntl.h>
#include <sre/game_object/light.h>
#include <sre/game_object/game_object_base.h>
#include <sre/transform/transform.h>
#include <SDL2/SDL.h>

typedef struct struct_sre_group
{
    sre_game_object_base base;
    sre_object_list components;
    sre_collider *collider;
    sre_transform transform;
}
sre_group;

typedef struct struct_sre_event sre_event;

void SRE_Group_create(sre_group **group, const char *name);
void SRE_Group_add_component(sre_group *group, sre_game_object *game_object);
void SRE_Group_remove_component(sre_group *group, uint16_t obj_id);
void SRE_Group_find_all_of_type(sre_group *group, sre_object_type type, sre_game_object **objects, int *found_count, int max_count);

typedef void (*SRE_Component_callback)(sre_game_object *game_object, void *args);
void SRE_Group_foreach_component(sre_group *group, uint32_t types, SRE_Component_callback callback, void *args);

void SRE_Group_load(sre_group *group);
void SRE_Group_unload(sre_group *group);
int SRE_Group_draw(sre_group *group, sre_program program, uint32_t types);
int SRE_Group_handle_event(sre_group *group, SDL_Event event);
int SRE_Group_handle_user_event(sre_group *group, sre_event event);


#endif //SRE_GROUP_H
