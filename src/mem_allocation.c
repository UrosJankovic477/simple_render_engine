#include "mem_allocation.h"

sre_mempool main_mempool;
sre_mempool aux_mempool; 

int SRE_Mempool_reset(sre_mempool *mempool_ptr)
{
    mempool_ptr->index = 0;
    return SRE_SUCCESS;
}

int SRE_Mempool_alloc(sre_mempool *mempool_ptr, void **dest, size_t size)
{
    if (size <= 0 || size + mempool_ptr->index > mempool_ptr->size)        
    {
        return SRE_ERROR_NO_MEM;
    }
    
    *dest = (char*)mempool_ptr->data + mempool_ptr->index;
    mempool_ptr->index += size;    
    return SRE_SUCCESS;
}

int SRE_Mempool_create(sre_mempool *parent_pool_ptr, sre_mempool *mempool_ptr, size_t size)
{
    if (parent_pool_ptr == NULL)
    {
        mempool_ptr->data = malloc(size);
        if (mempool_ptr->data == NULL)
        {
            return SRE_ERROR_NO_MEM;
        }
        
    }
    else {
        int status = SRE_Mempool_alloc(parent_pool_ptr, &mempool_ptr->data, size);
        if (status != SRE_SUCCESS)
        {
            return status;
        }
    }
    mempool_ptr->parent = parent_pool_ptr;
    mempool_ptr->index = 0;
    mempool_ptr->size = size;
    return SRE_SUCCESS;
}

int SRE_Mempool_destroy(sre_mempool *mempool_ptr)
{
    if (mempool_ptr->parent != NULL)
    {
        return SRE_ERROR;
    }
    free(mempool_ptr->data);
    mempool_ptr->data = NULL;
    mempool_ptr->index = -1;
    return SRE_SUCCESS;
}

