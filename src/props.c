#include "props.h"

void SRE_Prop_create(sre_mesh *mesh, sre_collider *collider, sre_prop *out_prop, sre_cntl_handle *cntl)
{ 
    out_prop->collider = collider;
    out_prop->cntl = cntl;
    if (cntl && collider)
    {
        SRE_Col_get_center(collider, out_prop->cntl->orig);
    }
    
    out_prop->mesh = mesh;
}

void SRE_Prop_draw(sre_prop prop, sre_program program, bool draw_collider)
{
    if (prop.mesh)
    {
        SRE_Mesh_draw(prop.mesh, program);
    }
    if (prop.collider && draw_collider)
    {
        SRE_Draw_collider(prop.collider, program);
    }
    
}

void SRE_Prop_translate(sre_prop *prop, vec3 xyz)
{
    glm_translate(prop->mesh->model_mat, xyz);
    if (prop->cntl)
    {
        glm_vec3_add(prop->cntl->orig, xyz, prop->cntl->orig);
    }
    if (prop->collider)
    {
        SRE_Col_translate(prop->collider, xyz);
    } 
}

void SRE_Prop_rotate(sre_prop *prop, vec3 axis, float angle)
{
    glm_rotate(prop->mesh->model_mat, angle, axis);
}

void SRE_Prop_rotate_x(sre_prop *prop, float angle)
{
    glm_rotate_x(prop->mesh->model_mat, angle, prop->mesh->model_mat);
}

void SRE_Prop_rotate_y(sre_prop *prop, float angle)
{
    glm_rotate_y(prop->mesh->model_mat, angle, prop->mesh->model_mat);
}

void SRE_Prop_rotate_z(sre_prop *prop, float angle)
{
    glm_rotate_z(prop->mesh->model_mat, angle, prop->mesh->model_mat);
}

void SRE_Prop_rotate_euler(sre_prop *prop, vec3 euler_rot)
{
    mat4 trans;
    glm_mat4_identity(trans);
    glm_euler(euler_rot, trans);
    glm_mat4_mul(prop->mesh->model_mat, trans, prop->mesh->model_mat);
}

void SRE_Prop_scale(sre_prop *prop, vec3 xyz)
{
    glm_scale(prop->mesh->model_mat, xyz);
}

void SRE_Prop_gen_aabb(sre_prop *prop, sre_mempool *mempool)
{
    SRE_Mempool_alloc(mempool, &prop->collider, sizeof(sre_collider));
    SRE_Mempool_alloc(mempool, &prop->collider->data, sizeof(sre_coldat_aabb));
    sre_coldat_aabb *data = prop->collider->data;
    prop->collider->type = COL_AABB;
    glm_vec3_zero(data->min_pt);
    glm_vec3_zero(data->max_pt);
    sre_mesh *mesh = prop->mesh;
    vec3 co;
    for (size_t i = 0; i < mesh->vertex_count; i++)
    {
        glm_vec3_copy(mesh->vertex_positions[i], co);
        for (size_t j = 0; j < 3; j++)
        {
            data->min_pt[i] = co[i] < data->min_pt[i] ? co[i] : data->min_pt[i];
            data->max_pt[i] = co[i] > data->max_pt[i] ? co[i] : data->max_pt[i];
        }
    }
}

void SRE_Prop_control(sre_prop *prop, float dt, float dx, float dy)
{
    SRE_Cntl_rotate(prop->cntl, dt, dx, dy);
    if (!(prop->cntl->flags ^ CTL_MOUSE_MOVEMENT))
    {
        return;
    }
    
    vec3 old_orig;
    glm_vec3_copy(prop->cntl->orig, old_orig);
    SRE_Cntl_translate(prop->cntl, dt);
    vec3 intersection, old_intersection, end, new_end;
    glm_vec3_copy(prop->cntl->orig, end);
    glm_vec3_copy(end, new_end);
    float min_dist = glm_vec3_distance(old_orig, end);
    for (size_t i = 0; i < col_count; i++)
    {
        if (prop->collider->col_queue_idx == i)
        {
            continue;
        }

        sre_collider *col_in_queue = col_queue[i];
        
        glm_vec3_copy(intersection, old_intersection);
        

        sre_collision_context context;
        context.type = col_in_queue->type;
        context.coldat = col_in_queue->data;
        context.bsp_tree_index = 0;
        context.moving_collider = prop->collider->data;
        glm_vec3_copy(end, context.end);

        bool collission = SRE_Line_intersection(old_orig, &context, SRE_Col_handler_solid);

        if (!collission)
        {
            continue;
        }
        float dist = glm_vec3_distance(old_orig, context.intersection);
        if (dist < min_dist)
        {
            min_dist = dist;
            glm_vec3_copy(context.end, new_end);
            glm_vec3_copy(context.intersection, end);
        }
    }

    vec3 trans;
    glm_vec3_copy(new_end, prop->cntl->orig);
    glm_vec3_sub(new_end, old_orig, trans);
    SRE_Col_translate(prop->collider, trans);
    glm_translate(prop->mesh->model_mat, trans);
}
