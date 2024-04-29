#ifndef SRE_MEM_ALLOCATION
#define SRE_MEM_ALLOCATION

#include <stdlib.h>

typedef struct struct_sre_mempool
{
    char *data;
    unsigned long long index;
    unsigned long long size;
} sre_mempool;


extern sre_mempool dflt_mempool;

int SRE_Init_mempool(sre_mempool *mempool, size_t size);
int SRE_Get_mem_from_mempool(sre_mempool mempool, void **dest, size_t size);
int SRE_Finalize_mempool(sre_mempool mempool_ptr);

#endif