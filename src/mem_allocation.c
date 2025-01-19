#include <sre/mem_allocation.h>

sre_bumpalloc main_allocator;

int SRE_Bump_reset(sre_bumpalloc *bump)
{
    bump->index = 0;
    bump->freelist.head = NULL;
    bump->freelist.tail = NULL;
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

    if (size <= bump->freelist.largest_free_size)
    {
        sre_freelist_entry *current_node = bump->freelist.head;
        sre_freelist_entry *prev_node = NULL;
        while (current_node && size > current_node->size)
        {
            prev_node = current_node;
            current_node = current_node->next;
        }
        if (prev_node == NULL)
        {
            *dest = (void*)bump->freelist.head;
            bump->freelist.head = bump->freelist.head->next;
            if (bump->freelist.head == NULL)
            {
                bump->freelist.tail = NULL;
                bump->freelist.largest_free_size = 0;
            }
            
            
        }
        else if (current_node != NULL)
        {
            prev_node->next = current_node->next;
            if (current_node->size == bump->freelist.largest_free_size)
            {
                bump->freelist.largest_free_size = ~0;
            }
            
            if (current_node->next == NULL)
            {
                bump->freelist.tail = prev_node;
            }
            
            *dest = current_node;
        }
        else
        {
            *dest = (uint8_t *)bump->data + bump->index;
            bump->index += size;
            return SRE_SUCCESS;
        }
        return SRE_SUCCESS;
    }

    // if there's no free space in freelist just do normal bump aloc

    *dest = (uint8_t *)bump->data + bump->index;
    bump->index += size;
    return SRE_SUCCESS;
}

int SRE_Bump_free(sre_bumpalloc *bump, void **block, size_t size)
{
    size_t size_mod16 = size & 0xf;
    if (size_mod16)
    {
        size += 16 - size_mod16;
    }

    if (*block + size == bump->index + bump->data)
    {
        bump->index -= size;
        *block = NULL;
        return SRE_SUCCESS;
    }

    

    if (bump->freelist.head == NULL)
    {
        bump->freelist.head = ((sre_freelist_entry*) *block);
        bump->freelist.tail = bump->freelist.head;
        bump->freelist.head->next = NULL;
        bump->freelist.head->size = size;
        bump->freelist.largest_free_size = size;
        return SRE_SUCCESS;
    }
    else
    {
        bump->freelist.tail->next = ((sre_freelist_entry*) *block);
        bump->freelist.tail->next->next = NULL;
        bump->freelist.tail->next->size = size;
        bump->freelist.tail = bump->freelist.tail->next;
        if (bump->freelist.largest_free_size == ~0 || bump->freelist.largest_free_size < size)
        {
            bump->freelist.largest_free_size = size;
        }
        
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
    bump->freelist.tail = NULL;
    return SRE_SUCCESS;
}

int SRE_Bump_destroy(sre_bumpalloc *bump)
{
    free(bump->data);
    bump->data = NULL;
    bump->index = -1;
    bump->freelist.head = NULL;
    bump->freelist.tail = NULL;
    return SRE_SUCCESS;
}
