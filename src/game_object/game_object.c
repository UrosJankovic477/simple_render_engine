#include <sre/game_object/game_object.h>
#include <cglm/call/mat4.h>
#include <sre/game_object/import_export/importer.h>

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
    uint16_t duplicate_count;
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

size_t SRE_Game_object_get_size(sre_object_type type)
{
    size_t size;
    switch (type)
    {
        case SRE_GO_ARMATURE:
            size = sizeof(sre_armature);
            break;
        
        case SRE_GO_MATERIAL:
            size = sizeof(sre_material);
            break;

        case SRE_GO_MESH:
            size = sizeof(sre_mesh);
            break;
        
        case SRE_GO_GROUP:
            size = sizeof(sre_group);
            break;

        case SRE_GO_TEXTURE:
            size = sizeof(sre_texture);
            break;
        
        case SRE_GO_LIGHT:
            size = sizeof(sre_light);
            break;

        case SRE_GO_CAMERA:
            size = sizeof(sre_camera);
            break;

        case SRE_GO_COLLIDER:
            size = sizeof(sre_collider);
            break;

        case SRE_GO_ASSET_INSANCE:
            size = sizeof(sre_asset_instance);
            break;

        default:
            break;
    }
    return size;
}

void SRE_Game_object_create_internal(char *name, sre_object_table_entry *node, sre_object_type type, uint16_t hash, sre_game_object **out_object_ptr)
{
    size_t size = SRE_Game_object_get_size(type);
    SRE_Bump_alloc(&main_allocator, (void**)&node->object, size);
    strncpy(node->object->base.name, name, 64);
    glmc_mat4_identity(node->object->base.parent_transform_mat);
    node->object->base.type = type;
    node->object->base.entry = NULL;
    node->object->base.id = hash;
    node->collision = 0xffff;
    *out_object_ptr = node->object;
}

int SRE_Game_object_create(const char *name, sre_object_type type, sre_game_object **out_object_ptr)
{
    if (go_manager.object_count == SRE_MAX_OBJECT_COUNT)
    {
        return SRE_ERROR;
    }
    
    uint16_t hash = get_string_hash16(name) & (uint16_t)(SRE_MAX_OBJECT_COUNT - 1u);
    sre_object_table_entry *node = &go_manager.object_table[hash];
    //if (node->object == NULL)
    //{
    //    SRE_Game_object_create_internal(name, node, type, hash, out_object_ptr);
    //    return SRE_SUCCESS;
    //}
    uint8_t i = 0;
    uint8_t old_hash;
    do
    {
        old_hash = hash;
        hash = (hash + i++) & (uint16_t)(SRE_MAX_OBJECT_COUNT - 1u); // hash % 256
        if (go_manager.object_table[hash].object == NULL)
        {
            go_manager.object_table[old_hash].collision = hash;
            node = &go_manager.object_table[hash];
            SRE_Game_object_create_internal(name, node, type, hash, out_object_ptr);
            go_manager.object_table[hash].duplicate_count = 0;
            go_manager.object_count++;

            return SRE_SUCCESS;
        }
        else if (strcmp(name, go_manager.object_table[hash].object->base.name) == 0)
        {
            go_manager.object_table[hash].duplicate_count++;
            char old_name[60];
            char new_name[64];
            strncpy(old_name, name, 60);
            snprintf(new_name, 64, "%s%03d", old_name);

            return SRE_Game_object_create(new_name, type, out_object_ptr);
        }
        
        
    }
    while (hash != old_hash);

    return SRE_ERROR;
}

int SRE_Game_object_remove(const char *name)
{
    uint16_t hash = get_string_hash16(name) & (uint16_t)(SRE_MAX_OBJECT_COUNT - 1u);
    uint16_t idx = (uint16_t)hash;
    while (idx != 0xffff)
    {
        if (go_manager.object_table[idx].object != NULL && strcmp(name, go_manager.object_table[idx].object->base.name) != 0)
        {
            break;
        }
        
        idx = go_manager.object_table[idx].collision;
    }
    if (idx == 0xffff)
    {
        return SRE_ERROR;
    }
    SRE_Bump_free(&main_allocator, &go_manager.object_table[idx].object, sizeof(sre_game_object));

    return SRE_SUCCESS;
}

int SRE_Game_object_get_id(uint16_t id, sre_game_object **object)
{
    if (go_manager.object_table[id].object == NULL || id > SRE_MAX_OBJECT_COUNT)
    {
        *object = SRE_GO_NULL;
        return SRE_ERROR;
    }
    *object = go_manager.object_table[id].object;
    return SRE_SUCCESS;
}

void SRE_Game_object_get_or_import(const char *name, const char *path, sre_game_object **object)
{
    int status = SRE_Game_object_get(name, object);
    if (status != SRE_SUCCESS)
    {
        SRE_Import_asset(path);
        SRE_Game_object_get(name, object);
    }
}

void SRE_Game_object_remove_id(uint16_t obj_id)
{
    size_t size = SRE_Game_object_get_size(go_manager.object_table[obj_id].object->base.type);
    SRE_Bump_free(&main_allocator, &go_manager.object_table[obj_id].object, size);
    go_manager.object_table[obj_id].object = NULL;
}