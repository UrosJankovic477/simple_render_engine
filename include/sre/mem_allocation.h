#ifndef SRE_MEM_ALLOCATION
#define SRE_MEM_ALLOCATION

#include <stdlib.h>
#include <stdint.h>
#include <sre/logging/errors.h>

typedef struct struct_sre_freelist_entry
{
    size_t size;
    struct struct_sre_freelist_entry *next;
}
sre_freelist_entry;

typedef struct struct_sre_freelist
{
    uint64_t length;
    size_t largest_free_block;
    sre_freelist_entry *head;
}
sre_freelist;

typedef struct struct_sre_bump
{
    uint8_t *data;
    uint64_t index;
    size_t size;
    sre_freelist freelist;
}
sre_bumpalloc;

extern sre_bumpalloc main_allocator;

int SRE_Bump_reset(sre_bumpalloc *bump);
int SRE_Bump_alloc(sre_bumpalloc *bump, void **dest, size_t size);
int SRE_Bump_free(sre_bumpalloc *bump, void **block, size_t size);
int SRE_Bump_create(sre_bumpalloc *bump, size_t size);
int SRE_Bump_destroy(sre_bumpalloc *bump);

#endif
