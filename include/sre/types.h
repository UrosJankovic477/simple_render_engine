#ifndef SRE_NORM_INT_H
#define SRE_NORM_INT_H

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <cglm/types.h>

#define FLOAT_BITS(x) *((unsigned int*)((void*)&(x)))
#define THREE10S_X(x) ((x) & 0x0000003f)
#define THREE10S_Y(x) (((x) >> 10) & 0x0000003f)
#define THREE10S_Z(x) (((x) >> 20) & 0x0000003f)
#define THREE10S_FLAGS(x) ((x) >> 30)
#define RGBA(r, g, b, a) ((int)(r) & 0xff) | (((int)(g) & 0xff) << 8) | (((int)(b) & 0xff) << 16) | (((int)(a) & 0xff) << 24)

typedef uint32_t sre_2_10_10_10s;

typedef uint16_t sre_norm_16;
typedef struct struct_sre_norm_16_vec2
{
    sre_norm_16 x;
    sre_norm_16 y;
}
sre_norm_16_vec2;

typedef enum enum_sre_color
{
    SRE_TRANSPARENT = 0,
    SRE_BLACK = 0x000000ff,
    SRE_WHITE = 0xffffffff,
    SRE_RED = 0xff0000ff,
    SRE_GREEN = 0x00ff00ff,
    SRE_BLUE = 0x0000ffff,
    SRE_YELLOW = 0xffff00ff,
    SRE_CYAN = 0x00ffffff,
    SRE_MAGENTA = 0xff00ffff,
    SRE_LIGHT_GRAY = 0x7f7f7fff,
    SRE_DARK_GRAY = 0x3f3f3fff,
}
sre_color;

typedef unsigned char sre_byte;
typedef union union_sre_rgba
{
    struct
    {
        sre_byte r;
        sre_byte g;
        sre_byte b;
        sre_byte a;
    };
    uint32_t rgba;
}
sre_rgba;

typedef union union_sre_euler
{
    struct
    {
        float pitch;
        float yaw;
        float roll;
    };
    vec3 vector;
}
sre_euler;

typedef struct sre_vtx_float_short_2_10_10_10_struct
{
    float posx;
    float posy;
    float posz;
    sre_norm_16 u;
    sre_norm_16 v;
    sre_2_10_10_10s normal;
    uint32_t bone1;
    uint32_t bone2;
    uint32_t bone3;
    uint32_t bone4;
    float w1;
    float w2;
    float w3;
    float w4;
}
sre_vertex, sre_vtx_float_short_2_10_10_10;

typedef struct sre_vtx_float_float_2_10_10_10_struct
{
    float posx;
    float posy;
    float posz;
    float u;
    float v;
    sre_2_10_10_10s normal;
}
sre_vtx_float_float_2_10_10_10;

typedef struct struct_sre_float_vec3
{
    float x;
    float y;
    float z;
}
sre_float_vec3;

#ifdef __cplusplus
extern "C" {
#endif
uint8_t SRE_Float_to_unorm_8(float flp);
uint16_t SRE_Float_to_unorm_16(float flp);
uint16_t SRE_Float_to_fixed_16(float flp);
uint16_t SRE_Cstrng_to_fixed(const char *cstr);
sre_2_10_10_10s SRE_Float_to_2_10_10_10(float flp_x, float flp_y, float flp_z);
sre_2_10_10_10s SRE_Float_to_2_10_10_10s(float flp_x, float flp_y, float flp_z);
sre_rgba SRE_Vec4_to_rgba(float r, float g, float b, float a);
sre_rgba SRE_Vec3_to_rgb(float r, float g, float b);
sre_rgba SRE_Float_to_rgb(float c);
float normalize_fixed(uint16_t fxp);
float fixed_to_float(uint16_t fxp);
#ifdef __cplusplus
}
#endif

#endif
