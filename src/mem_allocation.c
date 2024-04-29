#include "mem_allocation.h"

int SRE_Init_mempool(sre_mempool *mempool_ptr, size_t size)
{
    mempool_ptr->data = malloc(size);
    mempool_ptr->index = 0;
    mempool_ptr->size =  size;
    return 0;
}

int SRE_Get_mem_from_mempool(sre_mempool mempool, void **dest, size_t size)
{
    if (size <= 0 || size + mempool.index > mempool.size)        
    {
        return 1;
    }
    
    *dest = (char*)mempool.data + mempool.index;
    mempool.index += size;    
    return 0;
}

int SRE_Finalize_mempool(sre_mempool mempool_ptr)
{
    free(mempool_ptr.data);
    mempool_ptr.data = NULL;
    mempool_ptr.index = -1;
    return 0;
}
