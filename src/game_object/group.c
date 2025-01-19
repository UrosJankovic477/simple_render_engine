#include <sre/game_object/group.h>
#include <sre/mem_allocation.h>
#include <sre/game_object/game_object.h>
#include <sre/event/event.h>
#include <sre/game_object/import_export/importer.h>
#include <stdio.h>

static sre_scene scene;
static bool scene_init = false;

void SRE_Group_create(sre_group **group, const char *name)
{
    SRE_Game_object_create(name, SRE_GO_GROUP, (sre_game_object**)group);
    (*group)->loaded_incatnce_count = 0;
    (*group)->collider = NULL;
    (*group)->components.head = (*group)->components.tail = NULL;
    (*group)->components.count = 0;
    (*group)->components.parent_id = (*group)->base.id;
    SRE_Trans_identity(&(*group)->transform);
}

void SRE_Components_add(sre_list *components, sre_game_object *game_object)
{
    sre_list_entry *new_entry;
    SRE_Bump_alloc(&main_allocator, (void**)&new_entry, sizeof(sre_list_entry));
    components->count++;
    if (components->parent_id == 0xffff)
    {
        game_object->base.parent = (sre_internalnode*)&scene;
    }
    else
    {
        SRE_Game_object_get_id(components->parent_id, (sre_game_object**)&game_object->base.parent);
    }
    
    new_entry->list = components;
    new_entry->game_object = game_object;
    game_object->base.entry = new_entry;
    if (components->head == NULL)
    {
        new_entry->next = new_entry->prev = NULL;
        components->head = components->tail = new_entry;
    }
    else
    {
        new_entry->next = NULL;
        new_entry->prev = components->tail;
        components->tail->next = new_entry;
        components->tail = components->tail->next;
    }
}

void SRE_Group_add_component(sre_group *group, sre_game_object *game_object)
{
    game_object->base.parent = group;
    if (game_object->base.type == SRE_GO_COLLIDER)
    {
        group->collider = &game_object->collider;
        return;
    }
    SRE_Components_add(&group->components, game_object);
    
}

void SRE_Components_remove_component(sre_game_object *game_object)
{
    sre_list_entry *this_entry = game_object->base.entry;
    if (this_entry == NULL)
    {
        return;
    }
    game_object->base.entry = NULL;
    sre_list *list = this_entry->list;
    sre_list_entry *prev, *next;
    prev = this_entry->prev;
    if (prev == NULL)
    {
        list->head = list->head->next;
    }
    else
    {
        next = this_entry->next;
        prev->next = next;
    }

    list->count--;
    SRE_Bump_free(&main_allocator, &this_entry, sizeof(sre_list_entry));
}

void SRE_Group_find_all_of_type(sre_group *group, sre_object_type type, sre_game_object **objects, int *found_count, int max_count)
{
    int found = 0;
    sre_list_entry *current = group->components.head;
    while (current && found < max_count)
    {
        if (current->game_object->base.type == type)
        {
            objects[found++] = current->game_object;
        }
        current = current->next;
    }
}

void SRE_Group_destroy_callback(sre_group *group, void *dummy_arg)
{
    SRE_Group_destroy(group);
}

void SRE_Group_destroy(sre_group *group)
{
    SRE_Components_foreach(&group->components, SRE_GO_GROUP, SRE_Group_destroy_callback, NULL);
    sre_list_entry *current = group->components.head;
    while (current)
    {
        sre_list_entry *prev = current;
        current = current->next;
        prev->game_object->base.parent = NULL;
        prev->game_object->base.entry = NULL;
        SRE_Bump_free(&main_allocator, prev, sizeof(sre_list_entry));
    }
    SRE_Game_object_remove_id(group->base.id);
}


void SRE_Components_foreach(sre_list *components, uint32_t types, SRE_Component_callback callback, void *args)
{
    sre_list_entry *current = components->head;
    while (current)
    {
        if (current->game_object->base.type & (types))
        {
            callback(current->game_object, args);
        }
        current = current->next;
    }
}

typedef struct struct_sre_draw_callback_arg
{
    sre_program *program;
    uint32_t types;
}
sre_draw_callback_arg;

void SRE_Group_draw_callback(sre_game_object *object, sre_draw_callback_arg *arg)
{
    sre_program *program = arg->program;
    switch (object->base.type)
    {
    case SRE_GO_MESH:
    {
        SRE_Mesh_draw((sre_mesh*)object, program);

        break;
    }
    case SRE_GO_COLLIDER:
    {
        SRE_Collider_draw((sre_collider*)object, *program);
        break;
    }
    case SRE_GO_GROUP:
    {
        SRE_Group_draw((sre_group*)object, program, arg->types);
        break;
    }
    case SRE_GO_ASSET_INSANCE:
    {
        SRE_Asset_draw_insance((sre_asset_instance*)object, program, arg->types);
        break;
    }
    default:
        break;
    }
}

int SRE_Group_draw(sre_group *group, sre_program *program, uint32_t types)
{
    sre_draw_callback_arg arg;
    arg.program = program;
    arg.types = types;
    SRE_Components_foreach(&group->components, types, SRE_Group_draw_callback, &arg);
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
void SRE_Components_load_callback(sre_game_object *object, void *dummy_arg)
{
    switch (object->base.type)
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
    case SRE_GO_CAMERA:
    {
        if (!object->camera.lookat)
        {
            SRE_Game_object_get(object->camera.lookat_name, (sre_game_object**)&object->camera.lookat);
        }
        
        break;
    }
    case SRE_GO_GROUP:
    {
        SRE_Group_load((sre_group*)object, ~0);
        break;
    }
    case SRE_GO_ASSET_INSANCE:
    {
        SRE_Asset_instance_load((sre_asset_instance*)object);
        break;
    }
    default:
        break;
    }
}

void SRE_Components_unload_callback(sre_game_object *object, void *dummy_arg)
{
    switch (object->base.type)
    {
        case SRE_GO_MESH:
        {
            SRE_Mesh_unload(&object->mesh); 
            break;
        }

        case SRE_GO_COLLIDER:
        {
            SRE_Collider_unload(&object->collider);
            break;
        }
        case SRE_GO_GROUP:
        {
            SRE_Group_unload((sre_group*)object, ~0);
            break;
        }
        case SRE_GO_ASSET_INSANCE:
        {
            SRE_Asset_instance_unload((sre_asset*)object);
            break;
        }
        default:
            break;
    }
}

void SRE_Group_load(sre_group *group, uint32_t types)
{
    if ((types & SRE_GO_COLLIDER) && group->collider)
    {
        SRE_Collider_load(group->collider);
    }

    SRE_Components_foreach(&group->components, types, SRE_Components_load_callback, NULL);
}

void SRE_Group_unload(sre_group *group, uint32_t types)
{
    if ((types & SRE_GO_COLLIDER) && group->collider)
    {
        SRE_Collider_unload(group->collider);
    }

    SRE_Components_foreach(&group->components, types, SRE_Components_unload_callback, NULL);
}

void SRE_Group_collision_callback(sre_game_object *object, sre_transform_event *trans_event)
{
    switch (object->base.type)
    {
    case SRE_GO_GROUP:
    {
        sre_collision_event collision_event;
        SRE_Collider_trace_line(trans_event, &object->collider, &collision_event);
        break;
    }
    case SRE_GO_ASSET_INSANCE:
    {
        if (object->asset_insance.asset->base.type == SRE_GO_GROUP)
        {
            sre_collision_event collision_event;
            SRE_Collider_trace_line(trans_event, &object->collider, &collision_event);
        }
        break;
    }
    default:
        break;
    }
    
}

int SRE_Group_handle_transform(sre_group *group, sre_transform_event trans_event);

void SRE_Components_set_transform_callback(sre_game_object *object, mat4 *transform)
{
    glmc_mat4_copy(*transform, object->base.parent_transform_mat);
}

void SRE_Group_transfrorm_callback(sre_game_object *object, sre_transform_event *trans_event)
{
    switch (object->base.type)
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
        case SRE_GO_ASSET_INSANCE:
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
            break;
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
        SRE_Components_foreach(&group->components, SRE_GO_COLLIDER, SRE_Group_collision_callback, &trans_event);
    }
    SRE_Trans_copy(trans_event.end_trans, &group->transform);
    SRE_Components_foreach(&group->components, ~0, SRE_Group_transfrorm_callback, &trans_event);
    if (group->collider)
    {
        vec3 dv;
        glmc_vec3_sub(trans_event.end_trans.translation, trans_event.start_trans.translation, dv);
        SRE_Collider_translate(group->collider, dv);
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

void SRE_Scene_init()
{
    scene.base.entry = NULL;
    scene.base.type = SRE_GO_SCENE;  
    scene.base.id = 0xffff;
    scene.base.parent = NULL;
    glmc_mat4_identity(scene.base.parent_transform_mat);
    strcpy(scene.base.name, "SRE_SCENE");
    scene.components.head = scene.components.tail = NULL;
    scene.components.count = 0;
    scene.components.parent_id = 0xffff;
    scene_init = true;
}

void SRE_Scene_reset()
{
    if (!scene_init)
    {
        SRE_Scene_init();
    }
    else
    {
        SRE_Components_remove_component(main_camera);
        SRE_Game_object_remove_id(main_camera->base.id);
        main_camera = NULL;
    }
    
    
    scene.ambient.rgba = 0;
    scene.background_color.rgba = 0;
    sre_list_entry *current = scene.components.head;
    SRE_Components_foreach(&scene.components, ~0, SRE_Components_unload_callback, NULL);
    SRE_Components_foreach(&scene.components, SRE_GO_GROUP, SRE_Group_destroy_callback, NULL);
    while (current)
    {
        sre_list_entry* prev = current;
        current = current->next;
        SRE_Bump_free(&main_allocator, (void**)&prev, sizeof(sre_list_entry));
    }
    scene.components.head = scene.components.tail = NULL;
    scene.components.count = 0;
    
}

void SRE_Scene_add_component(sre_game_object *game_object)
{
    SRE_Components_add(&scene.components, game_object);
}

void SRE_Scene_draw(sre_program *program, uint32_t types)
{
    sre_draw_callback_arg arg;
    arg.program = program;
    arg.types = types;
    SRE_Components_foreach(&scene.components, types, SRE_Group_draw_callback, &arg);
}

void SRE_Write_name_internal(const char *object_name, FILE *file)
{
    static char name[66];
    strcpy(name, object_name);
    strcat(name, "\n");
    fwrite(name, sizeof(char), strlen(name), file);
}

void SRE_Write_path_internal(const char *object_path, FILE *file)
{
    static char path[258];
    strcpy(path, object_path);
    strcat(path, "\n");
    fwrite(path, sizeof(char), strlen(path), file);
}

void SRE_Group_export_to_file(sre_group *group, FILE *file);

void SRE_Components_export_to_file(sre_list list, FILE *file)
{
    fwrite(&list.count, sizeof(uint16_t), 1, file);
    sre_list_entry *current = list.head;
    while (current)
    {
        sre_game_object *go = current->game_object;
        uint8_t kw = SRE_KW_UNDEFINED;
        switch (go->base.type)
        {
            case SRE_GO_MESH:
            {
                kw = SRE_KW_MESH;
                fwrite(&kw, sizeof(uint8_t), 1, file);
                SRE_Write_name_internal(go->base.name, file);
                SRE_Write_path_internal(go->base.path, file);
                SRE_Write_name_internal(go->mesh.material_name, file);
                SRE_Write_path_internal(go->mesh.material_path, file);
                SRE_Write_name_internal(go->mesh.armature_name, file);
                if (go->mesh.armature_name[0] != '\0')
                {
                    SRE_Write_path_internal(go->mesh.armature_path, file);
                }

                break;
            }
            case SRE_GO_CAMERA :
            {
                kw = SRE_KW_CAMERA;
                fwrite(&kw, sizeof(uint8_t), 1, file);
                SRE_Write_name_internal(go->base.name, file);
                uint8_t lookat;
                if (go->camera.lookat)
                {
                    lookat = 1;
                    fwrite(&lookat, 1, 1, file);
                    SRE_Write_name_internal(go->camera.lookat->base.name, file);
                    fwrite(go->camera.lookat_offset, sizeof(float), 3, file);
                }
                else
                {
                    lookat = 0;
                    fwrite(&lookat, 1, 1, file);
                    fwrite(go->camera.direction, sizeof(float), 3, file);
                }

                fwrite(go->camera.translation, sizeof(float), 3, file);
                fwrite(&go->camera.asprat, sizeof(float), 1, file);
                fwrite(&go->camera.fovy, sizeof(float), 1, file);
                fwrite(&go->camera.nearz, sizeof(float), 1, file);
                fwrite(&go->camera.farz, sizeof(float), 1, file);
                
                break;
            }
            case SRE_GO_LIGHT:
            {
                kw = SRE_KW_LIGHT;
                fwrite(&kw, sizeof(uint8_t), 1, file);
                SRE_Write_name_internal(go->base.name, file);
                fwrite(&go->light.color.rgba, sizeof(uint32_t), 1, file);
                fwrite(&go->light.radius, sizeof(float), 1, file);
                fwrite(go->light.translation, sizeof(float), 3, file);
                fwrite(&go->light.type, 1, 1, file);

                break;
            }
            case SRE_GO_GROUP:
            {
                SRE_Group_export_to_file((sre_group*)go, file);
                break;
            }
            case SRE_GO_ASSET_INSANCE:
            {
                kw = SRE_KW_ASSET_INSTANCE;
                fwrite(&kw, 1, 1, file);
                SRE_Write_name_internal(go->asset_insance.asset->base.name, file);
                SRE_Write_path_internal(go->asset_insance.asset->base.path, file);
                fwrite(go->asset_insance.transform.translation, sizeof(float), 3, file);
                fwrite(go->asset_insance.transform.rotation.vector, sizeof(float), 3, file);
                fwrite(go->asset_insance.transform.scale, sizeof(float), 3, file);
                break;
            }

            default:
                break;
        }
        current = current->next;
    }
     
}

void SRE_Group_export_to_file(sre_group *group, FILE *file)
{
    uint8_t kw = SRE_KW_GROUP;
    fwrite(&kw, sizeof(uint8_t), 1, file);
    SRE_Write_name_internal(group->base.name, file);
    uint8_t collider_code = 0;
    if (group->collider)
    {
        collider_code |= 1;
        if (group->collider->type == SRE_COLLIDER_AABB)
        {
            collider_code |= 2;
            fwrite(&collider_code, 1, 1, file);
            SRE_Write_name_internal(group->collider->base.name, file);
            fwrite(group->collider->data.aabb.max_pt, sizeof(float), 3, file);
            fwrite(group->collider->data.aabb.min_pt, sizeof(float), 3, file);
            
        }
        else
        {
            fwrite(&collider_code, 1, 1, file);
            SRE_Write_name_internal(group->collider->base.name, file);
            SRE_Write_path_internal(group->collider->base.path, file);
        }
        
        
    }
    else
    {
        fwrite(&collider_code, 1, 1, file);
    }
    
    fwrite(group->transform.translation, sizeof(float), 3, file);
    fwrite(group->transform.rotation.vector, sizeof(float), 3, file);
    fwrite(group->transform.scale, sizeof(float), 3, file);
    
    SRE_Components_export_to_file(group->components, file);
}

int SRE_Scene_export(const char *filename)
{
    FILE *file = fopen(filename, "w");
    fwrite(&scene.ambient.rgba, sizeof(sre_rgba), 1, file);
    fwrite(&scene.background_color.rgba, sizeof(sre_rgba), 1, file);
    SRE_Components_export_to_file(scene.components, file);
    SRE_Write_name_internal(main_camera->base.name, file);
    SRE_Write_name_internal(main_control_listener.moving_object->base.name, file);
    printf("Scene saved in %s\n", filename);
    fclose(file);
}

void SRE_Components_import_from_file(sre_list *list, FILE *file);


sre_group * SRE_Group_import_from_file(FILE *file)
{
    sre_group *group;
    char buffer[256];
    fgets(buffer, 64, file);
    const char *name = strtok(buffer, blank_chars);
    SRE_Group_create(&group, name);
    uint8_t collider_code = 0;
    sre_collider *collider = NULL;
    fread(&collider_code, sizeof(uint8_t), 1, file);
    if (collider_code)
    {
        
        fgets(buffer, 64, file);
        char col_name[64];
        strcpy(col_name, strtok(buffer, blank_chars));
        int status = SRE_Game_object_get(name, &collider);
        if (status != SRE_SUCCESS)
        {
            if (collider_code & 2)
            {
                // AABB
                vec3 max_pt, min_pt;
                fread(max_pt, sizeof(float), 3, file);
                fread(min_pt, sizeof(float), 3, file);
                SRE_Collider_create_aabb(&collider, col_name, min_pt, max_pt);
            }
            else
            {
                // BSP Tree
                fgets(buffer, 256, file);
                const char *bsp_path = strtok(buffer, blank_chars);
                SRE_Import_asset(bsp_path);
                SRE_Game_object_get(col_name, &collider);
            }
        }
        else
        {
            if (collider_code & 2)
            {
                // AABB
                fseek(file, sizeof(float) * 6, SEEK_CUR);
            }
            else
            {
                // BSP Tree
                fgets(buffer, 256, file);
            } 
        }
    }

    group->collider = collider;
    fread(group->transform.translation, sizeof(float), 3, file);
    fread(group->transform.rotation.vector, sizeof(float), 3, file);
    fread(group->transform.scale, sizeof(float), 3, file);

    SRE_Collider_translate(group->collider, group->transform.translation);
    SRE_Components_import_from_file(&group->components, file);
    mat4 transform;
    SRE_Trans_create_mat(&group->transform, transform);
    SRE_Components_foreach(&group->components, ~0, SRE_Components_set_transform_callback, &transform);
    return group;
}

void SRE_Components_import_from_file(sre_list *list, FILE *file)
{
    uint16_t n;
    fread(&n, sizeof(uint16_t), 1, file);
    list->count = 0;
    uint8_t kw;
    for (size_t i = 0; i < n; i++)
    {
        sre_game_object *object;
        fread(&kw, sizeof(uint8_t), 1, file);
        switch (kw)
        {
            case SRE_KW_MESH:
            {
                char buffer[256];
                fgets(buffer, 64, file);
                char name[64];
                strcpy(name, strtok(buffer, blank_chars));
                fgets(buffer, 256, file);
                char path[256];
                strcpy(path, strtok(buffer, blank_chars));
                SRE_Game_object_get_or_import(name, path, &object);
                fgets(buffer, 64, file);
                char mtl_name[64];
                strcpy(mtl_name, strtok(buffer, blank_chars));
                fgets(buffer, 256, file);
                char mtl_path[256];
                strcpy(mtl_path, strtok(buffer, blank_chars));
                sre_game_object *mtl;
                SRE_Game_object_get_or_import(mtl_name, mtl_path, &mtl);
                object->mesh.material = mtl;
                strcpy(object->mesh.material_name, name);
                fgets(buffer, 64, file);
                char arm_name[64];
                char *buffer_strtok = strtok(buffer, blank_chars);
                
                if (buffer_strtok)
                {
                    sre_game_object *arm;
                    strcpy(arm_name, strtok(buffer, blank_chars));
                    fgets(buffer, 256, file);
                    const char *arm_path = strtok(buffer, blank_chars);
                    SRE_Game_object_get_or_import(arm_name, arm_path, &arm);
                    object->mesh.armature = arm;
                    strcpy(object->mesh.armature_name, name);
                }

                break;
            }
            case SRE_KW_CAMERA:
            {
                char buffer[256];
                fgets(buffer, 64, file);
                char cam_name[64];
                strcpy(cam_name, strtok(buffer, blank_chars));
                uint8_t lookat;
                fread(&lookat, sizeof(uint8_t), 1, file);
                sre_game_object *lookat_object = NULL;
                vec3 direction;
                vec3 lookat_offset;
                const char *lookat_name = NULL;
                if (lookat)
                {
                    fgets(buffer, 64, file);
                    lookat_name = strtok(buffer, blank_chars);
                    SRE_Game_object_get(lookat_name, &lookat_object);
                    fread(&lookat_offset, sizeof(float), 3, file);
                    glmc_vec3_zero(direction);
                }
                else
                {
                    glmc_vec3_zero(lookat_offset);
                    fread(direction, sizeof(float), 3, file);
                }
                
                vec3 translation;
                float asprat, fovy, nearz, farz;
                fread(translation, sizeof(float), 3, file);
                fread(&asprat, sizeof(float), 1, file);
                fread(&fovy, sizeof(float), 1, file);
                fread(&nearz, sizeof(float), 1, file);
                fread(&farz, sizeof(float), 1, file);

                SRE_Camera_create(&object, cam_name, translation, direction, nearz, farz, asprat, fovy);
                strcpy(object->camera.lookat_name, lookat_name);
                object->camera.lookat = lookat_object;
                glmc_vec3_copy(object->camera.lookat_offset, lookat_offset);
                SRE_Camera_set_proj_mat((sre_camera*)object);
                

                break;
            }
            case SRE_KW_LIGHT:
            {
                char buffer[64];
                fgets(buffer, 64, file);
                const char *light_name = strtok(buffer, blank_chars);
                uint32_t rgba;
                fread(&rgba, sizeof(uint32_t), 1, file);
                float radius;
                fread(&radius, sizeof(float), 1, file);
                vec3 translation;
                fread(translation, sizeof(float), 3, file);
                uint8_t type;
                fread(&type, sizeof(uint8_t), 1, file);
                sre_rgba color;
                color.rgba = rgba;
                SRE_Light_create((sre_light**)&object, light_name, translation, color, radius, type);

                break;
            }
            case SRE_KW_GROUP:
            {
                object = (sre_game_object*)SRE_Group_import_from_file(file);
                break;
            }
            case SRE_KW_ASSET_INSTANCE:
            {
                char buffer[256];
                fgets(buffer, 256, file);
                char name[64];
                strcpy(name, strtok(buffer, "\n"));
                fgets(buffer, 256, file);
                char path[256];
                strcpy(path, strtok(buffer, "\n"));
                sre_transform transform;
                fread(transform.translation, sizeof(float), 3, file);
                fread(transform.rotation.vector, sizeof(float), 3, file);
                fread(transform.scale, sizeof(float), 3, file);
                sre_game_object *asset;
                SRE_Game_object_get_or_import(name, path, &asset);
                SRE_Asset_create_instance(asset, transform, NULL, &object);
                
                break;
            }

            default:
                break;
        }
        SRE_Components_add(list, object);
    }
    

    return SRE_SUCCESS;
}

int SRE_Group_export(const char *filename, sre_group *group)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        return SRE_ERROR;
    }
    SRE_Group_export_to_file(group, file);
    strcpy(group->base.path, filename);
    fclose(file);
    return SRE_SUCCESS;
}

int SRE_Scene_import(const char *filename)
{
    SRE_Scene_reset();
    FILE *file = fopen(filename, "r");
    fread(&scene.ambient.rgba, sizeof(sre_rgba), 1, file);
    fread(&scene.background_color.rgba, sizeof(sre_rgba), 1, file);
    SRE_Components_import_from_file(&scene.components, file);
    char buffer[64];
    fgets(buffer, 64, file);
    const char *name = strtok(buffer, blank_chars);
    SRE_Game_object_get(name, (sre_game_object**)&main_camera);
    fgets(buffer, 64, file);
    name = strtok(buffer, blank_chars);
    fclose(file);
    SRE_Game_object_get(name, (sre_game_object**)&main_control_listener.moving_object);
    SRE_Components_foreach(&scene.components, ~0, SRE_Components_load_callback, NULL); 
    printf("scene imported\n");

    return SRE_SUCCESS;
}