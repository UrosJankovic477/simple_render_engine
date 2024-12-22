#ifndef SRE_EVENT_H
#define SRE_EVENT_H
#include <stdint.h>
#include <SDL2/SDL_events.h>
#include <cglm/call/affine.h>
#include <sre/logging/errors.h>
#include <sre/transform/transform.h>
#include <sre/game_object/game_object.h>
#include <sre/event/app_run.h>

#define SRE_EVENT_TYPE_COUNT

typedef enum enum_sre_event_type
{
    SRE_NULL_EVENT = 0,
    SRE_TRANSFORM_EVENT,
    SRE_COLLIDER_TRANSFORM_EVENT,
    SRE_COLLISION_EVENT,
    SRE_LAST_EVENT
}
sre_event_type;

typedef union union_sre_event_args
{
    sre_transform_event transform;
    sre_collision_event collision;
}
sre_event_args;

typedef struct struct_sre_event
{
    Sint32 code;
    sre_event_args *args;
    sre_game_object *sender;
}
sre_event;

#define SRE_NULL_EVENT (sre_event){SRE_NULL_EVENT, NULL}

int SRE_Event_to_SDL_event(sre_event event, SDL_Event *sdl_event);
int SRE_SDL_event_to_user_event(SDL_Event sdl_event, sre_event *usr_event);
int SRE_Event_handle(SDL_Event event);

#endif
