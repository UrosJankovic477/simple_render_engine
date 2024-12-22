#include <sre/game_object/group.h>
#include <sre/mem_allocation.h>
#include <sre/game_object/game_object.h>
#include <sre/event/event.h>

void SRE_Group_create(sre_group **group, const char *name)
{
    SRE_Game_object_create(name, (sre_game_object**)group);
    (*group)->base.go_type = SRE_GO_GROUP;
    (*group)->collider = NULL;
    (*group)->components.head = (*group)->components.tail = NULL;
    SRE_Trans_identity(&(*group)->transform);
}

void SRE_Group_add_component(sre_group *group, sre_game_object *game_object)
{
    if (game_object->base.go_type == SRE_GO_COLLIDER)
    {
        group->collider = &game_object->collider;
        return;
    }

    sre_object_list_entry *new_entry;
    SRE_Bump_alloc(&main_allocator, (void**)&new_entry, sizeof(sre_object_list_entry));
    game_object->base.parent = group;
    new_entry->game_object = game_object;
    if (group->components.head == NULL)
    {
        new_entry->next = new_entry->prev = NULL;
        group->components.head = group->components.tail = new_entry;
    }
    else
    {
        group->components.tail->next = new_entry;
    }
}

void SRE_Group_remove_component(sre_group *group, uint16_t obj_id)
{
    sre_object_list_entry *current = group->components.head;
    uint16_t current_id = group->components.head->game_object->base.id;
    while (current && current_id != obj_id)
    {
        current = current->next;
    }
    if (current)
    {
        sre_object_list_entry *current_next = current->next;
        current->prev->next = current_next;
    }


}

void SRE_Group_find_all_of_type(sre_group *group, sre_object_type type, sre_game_object **objects, int *found_count, int max_count)
{
    int found = 0;
    sre_object_list_entry *current = group->components.head;
    while (current && found < max_count)
    {
        if (current->game_object->base.go_type == type)
        {
            objects[found++] = current->game_object;
        }
        current = current->next;
    }
}

void SRE_Group_foreach_component(sre_group *group, uint32_t types, SRE_Component_callback callback, void *args)
{
    sre_object_list_entry *current = group->components.head;
    while (current)
    {
        if (current->game_object->base.go_type & types)
        {
            callback(current->game_object, args);
        }
        current = current->next;
    }
}

void SRE_Group_draw_callback(sre_game_object *object, sre_program *program)
{
    switch (object->base.go_type)
    {
    case SRE_GO_MESH:
    {
        SRE_Mesh_draw(&object->mesh, *program);

        break;
    }
    case SRE_GO_COLLIDER:
    {
        SRE_Collider_draw(&object->collider, *program);
        break;
    }
    default:
        break;
    }
}

int SRE_Group_draw(sre_group *group, sre_program program, uint32_t types)
{
    SRE_Group_foreach_component(group, types, SRE_Group_draw_callback, &program);
}

int SRE_Group_handle_event(sre_group *group, SDL_Event event)
{
    switch (event.type)
    {
        case SDL_USEREVENT:
        {
            sre_event usr_event;
            SRE_SDL_event_to_user_event(event, &usr_event);
            return SRE_Group_handle_user_event(group, usr_event);
        }

        default:
        {
            return SRE_UNRECOGNIZED_EVENT;
        }

    }


}
void SRE_Group_load_callback(sre_game_object *object, void *dummy_arg)
{
    switch (object->base.go_type)
    {
    case SRE_GO_MESH:
    {
        SRE_Mesh_load(&object->mesh);
        break;
    }

    case SRE_GO_COLLIDER:
    {
        SRE_Collider_load(&object->collider);
        break;
    }
    default:
        break;
    }
}

void SRE_Group_unload_callback(sre_game_object *object, void *dummy_arg)
{
    switch (object->base.go_type)
    {
    case SRE_GO_MESH:
        SRE_Mesh_unload(&object->mesh);

    case SRE_GO_COLLIDER:
        SRE_Collider_unload(&object->collider);
    default:
        break;
    }
}

void SRE_Group_load(sre_group *group)
{
    if (group->collider)
    {
        SRE_Collider_load(group->collider);
    }

    SRE_Group_foreach_component(group, ~0, SRE_Group_load_callback, NULL);
}

void SRE_Group_unload(sre_group *group)
{
    if (group->collider)
    {
        SRE_Collider_unload(group->collider);
    }

    SRE_Group_foreach_component(group, ~0, SRE_Group_unload_callback, NULL);
}

void SRE_Group_collision_callback(sre_game_object *object, sre_transform_event *trans_event)
{
    sre_collision_event collision_event;
    SRE_Collider_trace_line(trans_event, &object->collider, &collision_event);
}

int SRE_Group_handle_transform(sre_group *group, sre_transform_event trans_event);

void SRE_Group_transfrorm_callback(sre_game_object *object, sre_transform_event *trans_event)
{
    switch (object->base.go_type)
    {
        case SRE_GO_GROUP:
        {
            SRE_Group_handle_transform(&object->group, *trans_event);
            break;
        }
        case SRE_GO_CAMERA:
        {
            sre_transform transform;
            SRE_Trans_copy(trans_event->end_trans, &transform);
            glmc_vec3_negate(transform.rotation.vector);
            SRE_Trans_create_mat(&transform, object->base.parent_transform_mat);
            SRE_Camera_set_view_mat(&object->camera);
            break;
        }
        case SRE_GO_MESH:
        {
            sre_transform transform;
            SRE_Trans_copy(trans_event->end_trans, &transform);
            glmc_vec3_negate(transform.rotation.vector);
            transform.rotation.pitch = 0.0f;
            SRE_Trans_create_mat(&transform, object->base.parent_transform_mat);
            break;
        }
        case SRE_GO_LIGHT:
        {
            mat4 transform;
            SRE_Trans_create_mat(&trans_event->end_trans, transform);
            SRE_Light_transform(&object->light, transform);
        }
        default:
        {
            break;
        }
    }
}

int SRE_Group_handle_transform(sre_group *group, sre_transform_event trans_event)
{
    if (trans_event.trans_flags & SRE_TRANS_CHECK_COLLISION)
    {
        int found_count = 0;
        sre_collision_event collision_event;
        SRE_Collider_trace_line(&trans_event, group->collider, &collision_event);
        SRE_Group_foreach_component(group, SRE_GO_COLLIDER, SRE_Group_collision_callback, &trans_event);
    }
    SRE_Trans_copy(trans_event.end_trans, &group->transform);
    SRE_Group_foreach_component(group, ~0, SRE_Group_transfrorm_callback, &trans_event);
    if (group->collider)
    {
        SRE_Collider_translate(group->collider, trans_event.end_trans.translation);
    }


    return SRE_SUCCESS;
}

int SRE_Group_handle_user_event(sre_group *group, sre_event usr_event)
{
    switch (usr_event.code)
    {
        case SRE_TRANSFORM_EVENT:
        {
            return SRE_Group_handle_transform(group, usr_event.args->transform);
        }
        default:
        {
            return SRE_UNRECOGNIZED_EVENT;
        }
    }
}
