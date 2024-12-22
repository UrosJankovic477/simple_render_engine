#include <sre/mem_allocation.h>

sre_bumpalloc main_allocator;

int SRE_Bump_reset(sre_bumpalloc *bump)
{
    bump->index = 0;
    return SRE_SUCCESS;
}

int SRE_Bump_alloc(sre_bumpalloc *bump, void **dest, size_t size)
{
    if (size <= 0 || size + bump->index > bump->size)
    {
        return SRE_ERROR_NO_MEM;
    }
    size_t size_mod16 = size & 0xf;
    if (size_mod16)
    {
        size += 16 - size_mod16;
    }

    // checking freelist

    if (size <= bump->freelist.largest_free_block)
    {
        sre_freelist_entry *current_free_block = bump->freelist.head;
        sre_freelist_entry *prev_free_block = NULL;
        while (current_free_block)
        {
            if (current_free_block->size >= size)
            {
                *dest = (uint8_t *)current_free_block;
                sre_freelist_entry *next_free_block;
                if (size == current_free_block->size)
                {
                    next_free_block = current_free_block->next;
                }
                else
                {
                    next_free_block = current_free_block + size;
                    next_free_block->size = current_free_block->size - size;
                    next_free_block->next = current_free_block->next;
                }


                if (prev_free_block == NULL)
                {
                    bump->freelist.head = next_free_block;
                }
                else
                {
                    prev_free_block->next = next_free_block;
                }
                return SRE_SUCCESS;
            }

            prev_free_block = current_free_block;
            current_free_block = current_free_block->next;
        }
        return SRE_ERROR; // shouldn't be possible
    }

    // if there's no free space in freelist just do normal bump aloc

    *dest = (uint8_t *)bump->data + bump->index;
    bump->index += size;
    return SRE_SUCCESS;
}

int SRE_Bump_free(sre_bumpalloc *bump, void **block, size_t size)
{
    if (*block + size == bump->index)
    {
        bump -= size;
        *block = NULL;
        return SRE_SUCCESS;
    }

    sre_freelist_entry *next_node = bump->freelist.head;
    bump->freelist.head = (sre_freelist_entry *)(*block);
    bump->freelist.head->size = size;
    bump->freelist.head->next = next_node;
    if (size > bump->freelist.largest_free_block)
    {
        bump->freelist.largest_free_block = size;
    }

    *block = NULL;

    return SRE_SUCCESS;
}

int SRE_Bump_create(sre_bumpalloc *bump, size_t size)
{
    bump->data = malloc(size);
    if (bump->data == NULL)
    {
        return SRE_ERROR_NO_MEM;
    }

    bump->index = 0;
    bump->size = size;
    bump->freelist.head = NULL;
    bump->freelist.largest_free_block = 0;
    return SRE_SUCCESS;
}

int SRE_Bump_destroy(sre_bumpalloc *bump)
{
    free(bump->data);
    bump->data = NULL;
    bump->index = -1;
    bump->freelist.head = NULL;
    bump->freelist.largest_free_block = 0;
    return SRE_SUCCESS;
}
