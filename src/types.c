#include "types.h"
#include <stdio.h>

sre_norm_16 SRE_Float_to_norm_16(float flp)
{
    unsigned int ftoi = FLOAT_BITS(flp);
    unsigned int sign = ftoi & 0x80000000;
    
    unsigned int exponent = ftoi & 0x7f800000;
    unsigned int mantissa = ftoi & 0x007fffff;
    mantissa = mantissa | 0x00800000;
    mantissa = (mantissa >> 7) - 1;
    exponent = exponent >> 23;
    exponent -= 127;
    sre_norm_16 fp = mantissa;
    if (exponent & 0x80000000)
    {
        fp = fp >> -exponent;
    }
    else
    {
        fp = fp << exponent;
    }
    if (sign)
    {
        return -fp;
    }
    
    return fp;
}

sre_norm_16 SRE_Cstrng_to_fixed(const char *cstr)
{
    char cstr_cpy[19];
    char sign;
    if (cstr[0] == '-')
    {
        strcpy_s(cstr_cpy, 18, cstr + 1);
        sign = 1;
    }
    else
    {
        strcpy_s(cstr_cpy, 18, cstr);
        sign = 0;
    }
    const char *padding = "00000000";
    unsigned short fixed_upper = 0;
    unsigned short fixed_lower = 0;
    char *cstr_upper;
    char cstr_lower[9];
    cstr_upper = strtok(cstr_cpy, ".");
    strcpy_s(cstr_lower, 9, strtok(NULL, "\0"));
    strncat(cstr_lower, padding, 8 - strlen(cstr_lower));
    fixed_upper = atoi(cstr_upper);
    fixed_lower = 0x0000;
    unsigned int uint_lower = atoi(cstr_lower);
    unsigned short mask = 0x0080;
    for (unsigned char i = 0; (uint_lower != 0) && i < 8; i++)
    {
        uint_lower = uint_lower << 1;
        if (uint_lower >= 100000000u)
        {
            fixed_lower |= mask;
            uint_lower -= 100000000u;
        }
        mask = mask >> 1;
    }
    sre_norm_16 retval = (sre_norm_16) ((fixed_upper << 8) | fixed_lower);
    return sign == 0 ? retval : -retval;
}

sre_2_10_10_10s SRE_Float_to_2_10_10_10s(float flp_x, float flp_y, float flp_z)
{
    int32_t tenx = flp_x * 511;
    int32_t teny = flp_y * 511;
    int32_t tenz = flp_z * 511;
    int32_t three10s = (tenx & 0x000003ff) | ((teny & 0x000003ff) << 10) | ((tenz & 0x000003ff) << 20);
    return three10s;
}

sre_rgba SRE_Vec4_to_rgba(float r, float g, float b, float a)
{
    sre_rgba _rgba;
    _rgba.color_vec.r = (sre_byte)(0xff * r);
    _rgba.color_vec.g = (sre_byte)(0xff * g);
    _rgba.color_vec.b = (sre_byte)(0xff * b);
    _rgba.color_vec.a = (sre_byte)(0xff * a);
    return _rgba;
}

sre_rgba SRE_Vec3_to_rgb(float r, float g, float b)
{
    sre_rgba _rgba;
    _rgba.color_vec.r = (sre_byte)(0xff * r);
    _rgba.color_vec.g = (sre_byte)(0xff * g);
    _rgba.color_vec.b = (sre_byte)(0xff * b);
    _rgba.color_vec.a = 0xff;
    return _rgba;
}

sre_rgba SRE_Float_to_rgb(float c)
{
    sre_rgba _rgba;
    _rgba.color_vec.r = (sre_byte)(0xff * c);
    _rgba.color_vec.g = _rgba.color_vec.r;
    _rgba.color_vec.b = _rgba.color_vec.r;
    _rgba.color_vec.a = 0xff;
    return _rgba;
}

sre_2_10_10_10s SRE_Float_to_2_10_10_10(float flp_x, float flp_y, float flp_z)
{
    float floats[3] = {flp_x, flp_y, flp_z};
    sre_2_10_10_10s three10s = 0x00000000;
    for (size_t i = 0; i < 3; i++)
    {
        uint32_t bits = FLOAT_BITS(floats[i]);
        uint32_t mantissa = bits & 0x007fffff;
        uint32_t exponent = bits & 0x7f800000;
        mantissa = mantissa >> 13;
        exponent = exponent >> 23;
        exponent -= 127;
        unsigned int sign = bits & 0x80000000;
        sre_norm_16 one10 = mantissa;
        if (exponent & 0x80000000)
        {
            one10 = one10 >> -exponent;
        }
        else
        {
            one10 = one10 << exponent;
        }
        if (sign)
        {
            one10 = -one10;
        }
        one10 = one10 & 0x000003ff;
        three10s = three10s | (one10 << i * 10);
    }
    return three10s;
}

float fixed_to_float(sre_norm_16 fxp)
{
    float flp = 0;
    if (fxp)
    {
        int s = 1;
        if (fxp & 0x8000)
        {
            s = -1;
            fxp = -fxp;
        }
        flp = s * ((float)fxp / 256.0);
        return flp;
    }
    return flp;
}

/*
int main(int argc, char const *argv[])
{
    sre_norm_16s test_1 = cstrng_to_fixed("-1.5");
    sre_norm_16s test_2 = cstrng_to_fixed("23.25");
    sre_norm_16s test_3 = cstrng_to_fixed("3.14159265359");
    sre_norm_16s test_a = float_to_norm_16(-1.5f);
    sre_norm_16s test_b = float_to_norm_16(23.25f);
    sre_norm_16s test_c = float_to_norm_16(3.14159265359f);

    printf("test 1: %08x test a: %08x\n", test_1, test_a);
    printf("test 2: %08x test b: %08x\n", test_2, test_b);
    printf("test 3: %08x test c: %08x\n", test_3, test_c);

    return 0;
}
*/

