#include <sre/game_object/game_object.h>
#include <cglm/call/mat4.h>

void SRE_Queue_find_first_free(sre_queue *queue, uint16_t start_queue_index);

int SRE_Queue_enqueue(sre_queue *queue, sre_game_object *object)
{
    if (queue->first_free == SRE_QUEUE_SIZE - 1)
    {
        return SRE_ERROR;
    }

    queue->objects[queue->first_free] = object;
    object->base.queue_index = queue->first_free;
    if (queue->first_free == queue->last)
    {
        queue->last++;
        queue->first_free = queue->last;
    }
    else
    {
        SRE_Queue_find_first_free(queue, queue->first_free);
    }
    return SRE_SUCCESS;

}

void SRE_Queue_remove(sre_queue *queue, sre_game_object *object)
{
    queue->objects[object->base.queue_index] = NULL;
    if (object->base.queue_index < queue->first_free)
    {
        if (queue->first_free == queue->last)
        {
            queue->last = object->base.queue_index;
        }
        queue->first_free = object->base.queue_index;
    }
    object->base.queue_index = -1;
}

bool SRE_Queue_get_next(sre_queue *queue, sre_game_object **out_object)
{
    if (queue->next == queue->last)
    {
        *out_object = NULL;
        queue->next = 0;
        return false;
    }
    sre_game_object *current_object = queue->objects[queue->next++];
    while (current_object == NULL && queue->next < queue->last)
    {
        current_object = queue->objects[queue->next++];
    }
    *out_object = current_object;
    return true;
}

void SRE_Queue_find_first_free(sre_queue *queue, uint16_t start_queue_index)
{
    for (size_t i = start_queue_index; i < queue->last; i++)
    {
        if (queue->objects[i] == NULL)
        {
            queue->first_free = i;
            return;
        }
    }
    queue->first_free = queue->last;
}

typedef struct struct_sre_object_table_entry
{
    sre_game_object *object;
    uint16_t collision;
}
sre_object_table_entry;

typedef struct struct_sre_game_object_manager
{
    sre_object_table_entry object_table[SRE_MAX_OBJECT_COUNT];
    uint16_t object_count;
}
sre_game_object_manager;

static sre_game_object_manager go_manager;

int SRE_Game_object_manager_init()
{
    for (size_t i = 0; i < SRE_MAX_OBJECT_COUNT; i++)
    {
        SDL_zero(go_manager.object_table[i].object);
        go_manager.object_table[i].collision = 0xffff;
    }

    go_manager.object_count = 0;
}

int SRE_Game_object_get(const char *name, sre_game_object **object)
{
    uint16_t hash = get_string_hash16(name) & (uint16_t)(SRE_MAX_OBJECT_COUNT - 1u);
    uint16_t idx = (uint16_t)hash;
    while (go_manager.object_table[idx].object == NULL || strcmp(name, go_manager.object_table[idx].object->base.name) != 0)
    {
        idx = go_manager.object_table[idx].collision;
        if (idx == 0xffff )
        {
            break;
        }
    }
    if (idx == 0xffff)
    {
        *object = NULL;
        return SRE_ERROR;
    }
    if (go_manager.object_table[idx].object == NULL)
    {
        *object = NULL;
        return SRE_ERROR;
    }

    *object = go_manager.object_table[idx].object;

    return SRE_SUCCESS;
}

void Game_object_create_internal(char *name, sre_object_table_entry *node, uint16_t hash, sre_game_object **out_object_ptr)
{
    SRE_Bump_alloc(&main_allocator, (void**)&node->object, sizeof(sre_game_object));
    strncpy(node->object->base.name, name, 64);
    glmc_mat4_identity(node->object->base.parent_transform_mat);
    node->object->base.id = hash;
    node->collision = 0xffff;
    *out_object_ptr = node->object;
}

int SRE_Game_object_create(const char *name, sre_game_object **out_object_ptr)
{
    if (go_manager.object_count == SRE_MAX_OBJECT_COUNT)
    {
        return SRE_ERROR;
    }
    uint16_t hash = get_string_hash16(name) & (uint16_t)(SRE_MAX_OBJECT_COUNT - 1u);
    sre_object_table_entry *node = &go_manager.object_table[hash];
    if (node->object == NULL)
    {
        Game_object_create_internal(name, node, hash, out_object_ptr);
        return SRE_SUCCESS;
    }
    uint8_t i = 1;
    uint8_t old_hash = hash;
    do
    {
        hash = (hash + i++) & 0xff; // hash % 256
    }
    while (go_manager.object_table[hash].object != NULL || hash == old_hash);

    if (hash == old_hash)
    {
        return SRE_ERROR;
    }
    go_manager.object_table[old_hash].collision = hash;
    node = &go_manager.object_table[hash];
    Game_object_create_internal(name, node, hash, out_object_ptr);

    return SRE_SUCCESS;
}

int SRE_Game_object_remove(const char *name)
{
    uint16_t hash = get_string_hash16(name) & (uint16_t)(SRE_MAX_OBJECT_COUNT - 1u);
    uint16_t idx = (uint16_t)hash;
    while (go_manager.object_table[idx].object != NULL && strcmp(name, go_manager.object_table[idx].object->base.name) != 0 && idx != 0xffff)
    {
        idx = go_manager.object_table[idx].collision;
    }
    if (idx == 0xffff)
    {
        return SRE_ERROR;
    }
    SRE_Bump_free(&main_allocator, &go_manager.object_table[idx].object, sizeof(sre_game_object));

    return SRE_SUCCESS;
}

int SRE_Get_game_object_ptr_by_id(uint16_t id, sre_game_object **object)
{
    if (go_manager.object_table[id].object == NULL || id > SRE_MAX_OBJECT_COUNT)
    {
        *object = SRE_GO_NULL;
        return SRE_ERROR;
    }
    *object = &go_manager.object_table[id];
    return SRE_SUCCESS;
}
