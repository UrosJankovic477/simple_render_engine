#ifndef SRE_HASHMAP_H
#define SRE_HASHMAP_H
#include <stdint.h>

unsigned long int get_string_hash(unsigned char *string);
uint32_t get_string_hash32(unsigned char *string);
uint16_t get_string_hash16(unsigned char *string);
uint8_t get_string_hash8(unsigned char *string);


#endif