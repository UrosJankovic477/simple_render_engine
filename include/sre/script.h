#ifndef SRE_SCRIPT_H
#define SRE_SCRIPT_H

#include <sre/logging/errors.h>
#include <SDL2/SDL.h>
#include <string.h>

#define SRE_CMD_SIZE 256
#define SRE_UPDATE_BUFFER_SIZE 256

typedef int (*script_fn)(void *args);

typedef struct sre_script_struct
{
    char name[256];
    script_fn start;
    script_fn physics_update;
    script_fn update;
    void *so_handle;
} sre_scirpt;

script_fn cmds[SRE_CMD_SIZE];
sre_scirpt update_buffer[SRE_UPDATE_BUFFER_SIZE];

int SRE_Script_run(sre_scirpt script, void *args);
int SRE_Script_load(const char *script_name, sre_scirpt *script);
void SRE_Script_unload(sre_scirpt *script);

#endif //SRE_SCRIPT_H