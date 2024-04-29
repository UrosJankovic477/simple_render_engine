#ifndef SRE_COMMON_H
#define SRE_COMMON_H
#include "camera.h"
#include "cntl.h"
#include "errors.h"
#include "glad/glad.h"
#include "SDL2/SDL.h"
#include "light.h"
#include "model.h"
#include "props.h"
#include "postproc.h"
#include "shaders.h"
#include "texture.h"
#include "terrain_generation.h"
#include "types.h"

void SRE_Init(uint16_t width, uint16_t height, bool debug_on);
void main_loop();
void on_cntl();

#endif