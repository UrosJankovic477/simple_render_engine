#include "props.h"

void create_prop(gl_mesh *mesh, sre_collider *collider, mat4 *model, sre_prop *out_prop, sre_cntl_handle *cntl)
{
    if (model == NULL)
    {
        glm_mat4_identity(out_prop->model);
    }
    else
    {
        glm_mat4_copy(model, out_prop->model);
    }
    
    out_prop->collider = collider;
    out_prop->cntl = cntl;
    if (cntl && collider)
    {
        SRE_Col_get_center(collider, out_prop->cntl->orig);
    }
    
    out_prop->mesh = mesh;
}

void draw_prop(sre_prop prop, sre_program program)
{
    if (prop.mesh)
    {
        SRE_Draw_gl_mesh(prop.mesh, program, prop.model);
    }
    if (prop.collider)
    {
        SRE_Draw_collider(prop.collider, program);
    }
    
}

void translate_prop(sre_prop *prop, vec3 xyz)
{
    glm_translate(prop->model, xyz);
    if (prop->cntl)
    {
        glm_vec3_add(prop->cntl->orig, xyz, prop->cntl->orig);
    }
    if (prop->collider)
    {
        SRE_Col_translate(prop->collider, xyz);
    } 
}

void rotate_prop(sre_prop *prop, vec3 axis, float angle)
{
    glm_rotate(prop->model, angle, axis);
}

void rotate_prop_x(sre_prop *prop, float angle)
{
    glm_rotate_x(prop->mesh, angle, prop->mesh);
}

void rotate_prop_y(sre_prop *prop, float angle)
{
    glm_rotate_y(prop->mesh, angle, prop->mesh);
}

void rotate_prop_z(sre_prop *prop, float angle)
{
    glm_rotate_z(prop->mesh, angle, prop->mesh);
}

void rotate_euler_prop(sre_prop *prop, vec3 euler_rot)
{
    mat4 trans;
    glm_mat4_identity(trans);
    glm_euler(euler_rot, trans);
    glm_mat4_mul(prop->model, trans, prop->model);
}

void scale_prop(sre_prop *prop, vec3 xyz)
{
    glm_scale(prop->model, xyz);
}

void prop_control(sre_prop *prop, float dt, float dx, float dy)
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
        
        glm_vec3_copy(intersection, old_intersection);
        sre_collider expanded_col;
        sre_coldat_aabb col_data;
        expanded_col.data = &col_data;
        SRE_Expand_collision(prop->collider, col_queue[i], &expanded_col);
        bool collission = SRE_Line_intersection(old_orig, end, &expanded_col, intersection);
        if (!collission)
        {
            continue;
        }
        float dist = glm_vec3_distance(old_orig, intersection);
        if (dist < min_dist)
        {
            min_dist = dist;
            glm_vec3_copy(end, new_end);
            glm_vec3_copy(intersection, end);
        }
    }

    vec3 trans;
    glm_vec3_copy(new_end, prop->cntl->orig);
    glm_vec3_sub(new_end, old_orig, trans);
    SRE_Col_translate(prop->collider, trans);
    glm_translate(prop->model, trans);
}
