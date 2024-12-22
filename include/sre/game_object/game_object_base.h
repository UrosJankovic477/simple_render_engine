#ifndef SRE_GAME_OBJECT_BASE_H
#define SRE_GAME_OBJECT_BASE_H

#include <stdint.h>
#include <stdbool.h>
#include <sre/transform/transform.h>

#define SRE_QUEUE_SIZE 1028
#define SRE_QUEUE_FREELIST_SIZE (SRE_QUEUE_SIZE >> 6)

#ifndef SRE_GROUP_H
typedef struct struct_sre_group sre_group;
#endif

typedef enum enum_sre_object_type
{
    SRE_GO_NULL = 0,
    SRE_GO_MESH = 1,
    SRE_GO_ARMATURE = 2,
    SRE_GO_MATERIAL = 4,
    SRE_GO_TEXTURE = 8,
    SRE_GO_COLLIDER = 16,
    SRE_GO_CONTROL_LISTENER = 32,
    SRE_GO_GROUP = 64,
    SRE_GO_CAMERA = 128,
    SRE_GO_LIGHT = 256,
}
sre_object_type;

typedef union union_sre_game_object sre_game_object;

typedef struct struct_sre_object_list_entry_data
{
    sre_transform_constraint translation_limit;
    sre_transform_constraint rotation_limit;
    sre_transform_constraint scale_limit;
    uint16_t allowed_transform;
    uint16_t constrained_transform;
}
sre_object_list_entry_data;

typedef struct struct_sre_object_list_entry
{
    sre_game_object *game_object;
    struct struct_sre_object_list_entry *next;
    struct struct_sre_object_list_entry *prev;
}
sre_object_list_entry;

typedef struct struct_sre_object_list
{
    sre_object_list_entry *head;
    sre_object_list_entry *tail;
}
sre_object_list;

typedef struct struct_sre_group sre_group;

typedef struct struct_sre_object_base
{
    sre_object_type go_type;
    sre_group *parent;
    uint16_t id;
    uint16_t queue_index;
    char name[64];
    mat4 parent_transform_mat;
}
sre_game_object_base;

typedef struct struct_sre_queue
{
    sre_game_object *objects[SRE_QUEUE_SIZE];
    uint16_t last;
    uint16_t first_free;
    uint16_t next;
}
sre_queue;

#define SRE_EMPTY_QUEUE ((sre_queue){{NULL}, 0, 0, 0})

int SRE_Queue_enqueue(sre_queue *queue, sre_game_object *object);
void SRE_Queue_remove(sre_queue *queue, sre_game_object *object);
bool SRE_Queue_get_next(sre_queue *queue, sre_game_object **out_object);

#endif
