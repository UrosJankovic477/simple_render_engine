#include "hashmap.h"

unsigned long get_string_hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

uint32_t get_string_hash32(unsigned char *string)
{
    return get_string_hash(string) & 0xffffffff;
}

uint16_t get_string_hash16(unsigned char *string)
{
    return get_string_hash(string) & 0xffff;
}

uint8_t get_string_hash8(unsigned char *string)
{
    return get_string_hash(string) & 0xff;
}
