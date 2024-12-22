#include <sre/event/event.h>

static Uint32 user_event_type = 0xffffffff;

typedef struct struct_sre_event_listener_queue
{
    sre_game_object *listeners[128];
    uint8_t listener_count;
    uint8_t next_listener_idx;
}
sre_event_listener_queue;

int SRE_Event_to_SDL_event(sre_event usr_event, SDL_Event *sdl_event)
{
    if (user_event_type == 0xffffffff)
    {
        user_event_type = SDL_RegisterEvents(1);
    }
    SDL_zero(*sdl_event);
    sdl_event->type = user_event_type;
    sdl_event->user.code = usr_event.code;
    sdl_event->user.data1 = usr_event.args;
    sdl_event->user.data2 = usr_event.sender;

    return SRE_SUCCESS;
}

int SRE_SDL_event_to_user_event(SDL_Event sdl_event, sre_event *usr_event)
{
    usr_event->code = sdl_event.user.code;
    usr_event->args = sdl_event.user.data1;
    usr_event->sender = sdl_event.user.data2;
}

int SRE_Event_handle(SDL_Event event)
{
    switch (event.type)
    {
    case SDL_QUIT:
    {
        SRE_App_quit();
        break;
    }
    case SDL_KEYDOWN:
    {
        break;
    }

    default:
        break;
    }
    return 0;
}
