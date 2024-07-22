#ifndef SRE_MEM_ALLOCATION
#define SRE_MEM_ALLOCATION

#include <stdlib.h>
#include <stdint.h>
#include "errors.h"

typedef struct struct_sre_mempool
{
    char *data;
    uint64_t index;
    size_t size;
    struct struct_sre_mempool *parent;
} sre_mempool;

extern sre_mempool main_mempool;
extern sre_mempool aux_mempool; // auxilary

int SRE_Mempool_reset(sre_mempool *mempool_ptr);
int SRE_Mempool_alloc(sre_mempool *mempool_ptr, void **dest, size_t size);
int SRE_Mempool_create(sre_mempool *parent_pool_ptr, sre_mempool *mempool_ptr, size_t size);
int SRE_Mempool_destroy(sre_mempool *mempool_ptr);

#endif