#ifndef SRE_CONTROLABLE_H
#define SRE_CONTROLABLE_H

#include <cglm/call/mat4.h>
#include <stdbool.h>
#include <sre/transform/transform.h>
#include <sre/logging/errors.h>
#include <sre/game_object/game_object_base.h>
#include <SDL2/SDL.h>

#define SRE_MAX_NUM_CNTL_HANDLERS 64

SDL_EventFilter extern control_event_filter;

typedef enum enum_sre_control_action
{
    SRE_CONTROL_ACTION_FORWARD,
    SRE_CONTROL_ACTION_BACK,
    SRE_CONTROL_ACTION_LEFT,
    SRE_CONTROL_ACTION_RIGHT,
    SRE_CONTORL_ACTION_UP,
    SRE_CONTROL_ACTION_DOWN,
    SRE_CONTROL_ACTION_INTERACT,
    SRE_CONTROL_ACTION_USE,
    SRE_CONTROL_ACTION_COUNT
}
sre_control_action;

typedef int (*sre_control_handler)(SDL_Event event, float dt);

typedef struct struct_sre_control_listener
{
    sre_group *moving_object;
    sre_control_handler control_handler;
    float velocity;
    float sensitivity;
    float distance;
    sre_orientation orientation;
    SDL_Scancode keyboard_config[SRE_CONTROL_ACTION_COUNT];
    vec2 mouse_motion;
    uint32_t control_flags;
    Uint8 mouse_config[3];
}
sre_control_listener;

extern sre_control_listener main_control_listener;

void SRE_Control_create(sre_control_listener *listener, float speed, float sensitivity, sre_control_handler control_handler);
int SRE_Control_handler_default(SDL_Event event, float dt);
void SRE_Control_update(sre_control_listener *listenter);
void SRE_Controls_handler(SDL_Event event, float dt);
void SRE_Control_listener_set_main(sre_control_listener *listener);

#endif
