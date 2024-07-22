#ifndef SRE_COMMON_H
#define SRE_COMMON_H
#include "camera.h"
#include "cntl.h"
#include "errors.h"
#include "glad/glad.h"
#include "SDL2/SDL.h"
#include "light.h"
#include "props.h"
#include "postproc.h"
#include "shaders.h"
#include "texture.h"
#include "terrain_generation.h"
#include "types.h"
#include "asset_import.h"

void SRE_Init(int *argc, char **argv[], sre_importer *importer);
void main_loop();
void on_cntl();

#endif