#include <sre/game_object/collision.h>
#include <sre/logging/errors.h>
#include <sre/event/event.h>
#include <sre/transform/transform.h>

static sre_queue collider_queue = SRE_EMPTY_QUEUE;

enum
{
    BSP_EMPTY = 0xffff,
    BSP_SOLID = 0xfffe,
};

int SRE_Collider_create_aabb(sre_collider **collider, const char *name, vec3 min_pt, vec3 max_pt)
{
    int status = SRE_Game_object_create(name, SRE_GO_COLLIDER, (sre_game_object**)collider);
    if (status != SRE_SUCCESS)
    {
        return status;
    }
    
    (*collider)->type = SRE_COLLIDER_AABB;
    glmc_vec3_copy(max_pt, (*collider)->data.aabb.max_pt);
    glmc_vec3_copy(min_pt, (*collider)->data.aabb.min_pt);


    return EXIT_SUCCESS;
}

bool SRE_Collider_test(sre_collider *col_1, sre_collider *col_2)
{
    switch (col_1->type | (col_2->type << 1))
    {
    case SRE_COLLIDER_AABB | (SRE_COLLIDER_AABB << 1):
        return SRE_Collider_test_aabbs(&col_1->data.aabb, &col_2->data.aabb);
    default:
        break;
    }

    return false;
}

int SRE_Copy_collider(sre_collider *src, sre_collider *dest)
{
    size_t size;
    switch (src->type)
    {
    case SRE_COLLIDER_AABB:
        size = sizeof(sre_aabb);
        break;
    default:
        return EXIT_FAILURE;
    }
    dest->type = src->type;
    memcpy(&dest->data, &src->data, size);
    return EXIT_SUCCESS;
}

bool SRE_Line_intersection_bsp_tree(vec3 start, sre_collision_event *event)
{
    uint16_t index = event->bsp_tree_index;
    if (index == BSP_EMPTY)
    {
        return false;
    }

    if (index == BSP_SOLID)
    {
        glmc_vec3_copy(start, event->intersection);
        return true;
    }
    sre_bsp_node *root = (sre_bsp_node *)event->other_collider->data.bsp_tree.tree;
    sre_bsp_node node = root[index];
    vec3 normal;
    glmc_vec3_copy(node.deviding_plane, normal);
    float d = node.deviding_plane[3];
    vec3 dx;
    glmc_vec3_sub(event->end, start, dx);
    float t1 = glmc_vec3_dot(start, normal);
    float t2 = glmc_vec3_dot(event->end, normal);

    uint8_t start_side = t1 <= d;
    uint8_t end_side = t2 <= d;


    if (start_side == end_side)
    {
        event->bsp_tree_index = node.children[start_side];
        return SRE_Line_intersection_bsp_tree(start, event);
    }

    vec3 mid;

    float t = (t1 - d) / (t1 - t2);

    if (t > 0)
    {
        t -= 0.2f;
    }
    else
    {
        t += 0.2f;
    }

    glmc_vec3_scale(dx, t, mid);
    glmc_vec3_add(mid, start, mid);

    glmc_vec3_copy(normal, event->normal);
    event->bsp_tree_parent = index;

    event->bsp_tree_index = node.children[start_side];
    vec3 old_end;
    glmc_vec3_copy(event->end, old_end);
    glmc_vec3_copy(mid, event->end);
    bool result = SRE_Line_intersection_bsp_tree(start, event);
    if (result)
    {
        return result;
    }
    glmc_vec3_copy(old_end, event->end);
    event->bsp_tree_index = node.children[end_side];
    return SRE_Line_intersection_bsp_tree(mid, event);
}

bool SRE_Line_intersection_aabb(vec3 start, sre_collision_event *event)
{
    uint8_t start_flags, end_flags, intersection_flags;
    sre_aabb data = event->other_collider->data.aabb;
    start_flags = (start[0] <= data.min_pt[0])
    | ((start[0] >= data.max_pt[0]) << 1)
    | ((start[1] <= data.min_pt[1]) << 2)
    | ((start[1] >= data.max_pt[1]) << 3)
    | ((start[2] <= data.min_pt[2]) << 4)
    | ((start[2] >= data.max_pt[2]) << 5);
    end_flags = (event->end[0] <= data.min_pt[0])
    | ((event->end[0] >= data.max_pt[0]) << 1)
    | ((event->end[1] <= data.min_pt[1]) << 2)
    | ((event->end[1] >= data.max_pt[1]) << 3)
    | ((event->end[2] <= data.min_pt[2]) << 4)
    | ((event->end[2] >= data.max_pt[2]) << 5);

    vec3 normals[6] = {
        {1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f},
    };

    if (!(start_flags | end_flags) || !start_flags)
    {
        // line fully or partially inside collider
        vec3 dv;
        glmc_vec3_sub(start, event->end, dv);
        float t[6];
        float proj[6];
        vec3 start_to_corner[2];

        glmc_vec3_sub(start, data.min_pt, start_to_corner[0]);
        glmc_vec3_sub(start, data.max_pt, start_to_corner[1]);

        float closest = 1.0f;
        uint8_t j;

        for (uint8_t i = 0; i < 6; i++)
        {
            uint8_t idx = i & 1;
            proj[i] = glmc_vec3_dot(normals[i], dv);
            t[i] = glmc_vec3_dot(normals[i], start_to_corner[idx]) / -proj[i];
            if (fabsf(t[i]) < fabsf(closest))
            {
                closest = t[i];
                j = i;
            }
        }
        vec3 dvt;
        glmc_vec3_scale(dv, closest, dvt);
        glmc_vec3_add(start, dvt, event->intersection);
        if (proj[j] > 0 && end_flags)
        {
            return false;
        }
        vec3 penetration, backtrack;
        glmc_vec3_sub(event->end, event->intersection, penetration);
        glmc_vec3_scale(normals[j], proj[j], backtrack);

        glmc_vec3_sub(event->end, proj, event->end);
        for (size_t i = 0; i < 3; i++)
        {
            event->end[i] = glm_clamp(event->end[i], data.min_pt[i] - 0.01f, data.max_pt[i] + 0.01f);
        }
        return true;
    }
    if (start_flags & end_flags)
    {
        // trivially denied
        return false;
    }
    // ->    ->       ->
    // x_  = x0 + t * dv
    // ->    ->   ->
    // dv  = x1 - x0

    vec3 dv;
    glmc_vec3_sub(start, event->end, dv);

    float t = 0.0f;

    glmc_vec3_copy(start, event->intersection);
    intersection_flags = start_flags;
    uint8_t hit_dir = -1;
    if (intersection_flags & 0x1)
    {
        // right
        float xmin = data.min_pt[0];
        t = (xmin - start[0]) / dv[0];
        event->intersection[0] = xmin - 0.01;
        event->intersection[1] += dv[1] * t;
        event->intersection[2] += dv[2] * t;
        intersection_flags = ((event->intersection[1] < data.min_pt[1]) << 2)
        | ((event->intersection[1] > data.max_pt[1]) << 3)
        | ((event->intersection[2] < data.min_pt[2]) << 4)
        | ((event->intersection[2] > data.max_pt[2]) << 5);
        hit_dir = 0;
    }
    else if (intersection_flags & 0x2)
    {
        // left
        float xmax = data.max_pt[0];
        t = (start[0] - xmax) / dv[0];
        event->intersection[0] = xmax + 0.01;
        event->intersection[1] += dv[1] * t;
        event->intersection[2] += dv[2] * t;
        intersection_flags = ((event->intersection[1] < data.min_pt[1]) << 2)
        | ((event->intersection[1] > data.max_pt[1]) << 3)
        | ((event->intersection[2] < data.min_pt[2]) << 4)
        | ((event->intersection[2] > data.max_pt[2]) << 5);
        hit_dir = 1;
    }
    if (intersection_flags & 0x4)
    {
        // up
        float ymin = data.min_pt[1];
        t = (ymin - event->intersection[1]) / dv[1];
        event->intersection[0] += dv[0] * t;
        event->intersection[1] = ymin - 0.01;
        event->intersection[2] += dv[2] * t;
        intersection_flags = ((event->intersection[0] < data.min_pt[0]))
        | ((event->intersection[0] > data.max_pt[0]) << 1)
        | ((event->intersection[2] < data.min_pt[2]) << 4)
        | ((event->intersection[2] > data.max_pt[2]) << 5);
        hit_dir = 2;
    }
    else if (intersection_flags & 0x8)
    {
        // down
        float ymax = data.max_pt[1];
        t = (event->intersection[1] - ymax) / dv[1];
        event->intersection[0] += dv[0] * t;
        event->intersection[1] = ymax + 0.01;
        event->intersection[2] += dv[2] * t;
        intersection_flags = ((event->intersection[0] <= data.min_pt[0]))
        | ((event->intersection[0] >= data.max_pt[0]) << 1)
        | ((event->intersection[2] <= data.min_pt[2]) << 4)
        | ((event->intersection[2] >= data.max_pt[2]) << 5);
        hit_dir = 3;
    }
    if (intersection_flags & 0x10)
    {
        // front
        float zmin = data.min_pt[2];
        t = (zmin - event->intersection[2]) / dv[2];
        event->intersection[0] += dv[0] * t;
        event->intersection[1] += dv[1] * t;
        event->intersection[2] = zmin - 0.01;
        intersection_flags = ((event->intersection[0] <= data.min_pt[0]))
        | ((event->intersection[0] >= data.max_pt[0]) << 1)
        | ((event->intersection[1] <= data.min_pt[1]) << 2)
        | ((event->intersection[1] >= data.max_pt[1]) << 3);
        hit_dir = 4;
    }
    else if (intersection_flags & 0x20)
    {
        // back
        float zmax = data.max_pt[2];
        t = (event->intersection[2] - zmax) / dv[2];
        event->intersection[0] += dv[0] * t;
        event->intersection[1] += dv[1] * t;
        event->intersection[2] = zmax + 0.01;
        intersection_flags = ((event->intersection[0] <= data.min_pt[0]))
        | ((event->intersection[0] >= data.max_pt[0]) << 1)
        | ((event->intersection[1] <= data.min_pt[1]) << 2)
        | ((event->intersection[1] >= data.max_pt[1]) << 3);
        hit_dir = 5;
    }

    glmc_vec3_copy(normals[hit_dir], event->normal);

    return true;
}

bool SRE_Line_intersection_internal(sre_transform_event *trans_event, sre_collider *moving_collider, sre_collider *other_collider, sre_event *collision_event)
{
    if (moving_collider->base.id == other_collider->base.id )
    {
        return false;
    }

    vec3 plane_normal;
    bool intersects = false;
    vec3 start;
    glmc_vec3_copy(trans_event->start_trans.translation, start);
    glmc_vec3_copy(trans_event->end_trans.translation, collision_event->args->collision.end);
    collision_event->args->collision.moving_collider = moving_collider;

    if (other_collider->type == SRE_COLLIDER_BSP_TREE)
    {
        collision_event->args->collision.other_collider = other_collider;
        collision_event->args->collision.bsp_tree_parent = -1;
        collision_event->args->collision.bsp_tree_index = 0;
        intersects = SRE_Line_intersection_bsp_tree(start, &collision_event->args->collision);
    }
    else if (other_collider->type == SRE_COLLIDER_AABB)
    {
        sre_aabb old_aabb;
        SDL_memcpy(&old_aabb, &other_collider->data.aabb, sizeof(sre_aabb));
        SRE_Collider_expand(moving_collider->data.aabb, other_collider);
        collision_event->args->collision.other_collider = other_collider;
        intersects = SRE_Line_intersection_aabb(start, &collision_event->args->collision);
        SDL_memcpy(&other_collider->data.aabb, &old_aabb, sizeof(sre_aabb));
        collision_event->args->collision.other_collider = other_collider;
    }

    return intersects;
}

void SRE_Collider_trace_line(sre_transform_event *trans_event, sre_collider *collider, sre_collision_event *out_event)
{
    sre_game_object *next_collider;
    sre_event collision_event;
    sre_collision_event args;
    collision_event.args = &args;
    bool collision = false;
    float min_dist = glmc_vec3_distance(trans_event->start_trans.translation, trans_event->end_trans.translation);
    while (SRE_Queue_get_next(&collider_queue, &next_collider))
    {
        if(!SRE_Line_intersection_internal(trans_event, collider, &next_collider->collider, &collision_event))
        {
            continue;
        }
        glmc_vec3_copy(collision_event.args->collision.end, trans_event->end_trans.translation);
        collision = true;
    }
    if (collision)
    {
        SRE_Collision_handler_solid(&collision_event.args->collision);
        glmc_vec3_copy(collision_event.args->collision.end, trans_event->end_trans.translation);
    }
}

int SRE_Collider_load(sre_collider *col)
{
    return SRE_Queue_enqueue(&collider_queue, (sre_game_object*)col);
}

void SRE_Collider_unload(sre_collider *col)
{
    SRE_Queue_remove(&collider_queue, (sre_game_object*)col);
}

void SRE_Collider_translate_aabb(sre_aabb *aabb, vec3 xyz)
{
    glmc_vec3_add(aabb->min_pt, xyz, aabb->min_pt);
    glmc_vec3_add(aabb->max_pt, xyz, aabb->max_pt);
}

void SRE_Collider_translate(sre_collider *col, vec3 xyz)
{
    switch (col->type)
    {
        case SRE_COLLIDER_AABB:
        {
            SRE_Collider_translate_aabb(&col->data.aabb, xyz);
            break;
        }

        default:
        {
            break;
        }
    }
}

bool SRE_Aabb_from_mesh(sre_mesh mesh, sre_collider **collider)
{
    char name[64];
    strncpy(name, mesh.base.name, 58);
    strcat(name, "_aabb");
    SRE_Game_object_create(name, SRE_GO_COLLIDER, (sre_game_object**)collider);
    (*collider)->type = SRE_COLLIDER_AABB;
    glmc_vec3_copy(mesh.vertex_positions[0], (*collider)->data.aabb.min_pt);
    glmc_vec3_copy(mesh.vertex_positions[0], (*collider)->data.aabb.max_pt);
    for (size_t i = 0; i < mesh.vertex_count; i++)
    {
        if (mesh.vertex_positions[i][0] > (*collider)->data.aabb.max_pt[0])
        {
            (*collider)->data.aabb.max_pt[0] = mesh.vertex_positions[i][0];
        }
        if (mesh.vertex_positions[i][1] > (*collider)->data.aabb.max_pt[1])
        {
            (*collider)->data.aabb.max_pt[1] = mesh.vertex_positions[i][1];
        }
        if (mesh.vertex_positions[i][2] > (*collider)->data.aabb.max_pt[2])
        {
            (*collider)->data.aabb.max_pt[2] = mesh.vertex_positions[i][2];
        }
        if (mesh.vertex_positions[i][0] < (*collider)->data.aabb.min_pt[0])
        {
            (*collider)->data.aabb.min_pt[0] = mesh.vertex_positions[i][0];
        }
        if (mesh.vertex_positions[i][1] < (*collider)->data.aabb.min_pt[1])
        {
            (*collider)->data.aabb.min_pt[1] = mesh.vertex_positions[i][1];
        }
        if (mesh.vertex_positions[i][2] < (*collider)->data.aabb.min_pt[2])
        {
            (*collider)->data.aabb.min_pt[2] = mesh.vertex_positions[i][2];
        }
    }
    return true;
}

//void SRE_Collider_get_center(sre_collider *col, vec3 center)
//{
//    sre_aabb *data = (sre_aabb*)col->data;
//    glmc_vec3_add(data->max_pt, data->min_pt, center);
//    glmc_vec3_scale(center, 0.5, center);
//}

void SRE_Collider_draw(sre_collider *col, sre_program program)
{
    GLint vao;
    GLint vbo;
    sre_aabb *data = &col->data.aabb;
    GLfloat buf[]  = {
        // x+
        data->max_pt[0], data->max_pt[1], data->max_pt[2],
        data->max_pt[0], data->min_pt[1], data->max_pt[2],

        data->max_pt[0], data->min_pt[1], data->max_pt[2],
        data->max_pt[0], data->min_pt[1], data->min_pt[2],

        data->max_pt[0], data->min_pt[1], data->min_pt[2],
        data->max_pt[0], data->max_pt[1], data->min_pt[2],

        data->max_pt[0], data->max_pt[1], data->min_pt[2],
        data->max_pt[0], data->max_pt[1], data->max_pt[2],

        // y+
        data->max_pt[0], data->max_pt[1], data->max_pt[2],
        data->min_pt[0], data->max_pt[1], data->max_pt[2],

        data->min_pt[0], data->max_pt[1], data->max_pt[2],
        data->min_pt[0], data->max_pt[1], data->min_pt[2],

        data->min_pt[0], data->max_pt[1], data->min_pt[2],
        data->max_pt[0], data->max_pt[1], data->min_pt[2],

        data->max_pt[0], data->max_pt[1], data->min_pt[2],
        data->max_pt[0], data->max_pt[1], data->max_pt[2],

        // z+
        data->max_pt[0], data->max_pt[1], data->max_pt[2],
        data->min_pt[0], data->max_pt[1], data->max_pt[2],

        data->min_pt[0], data->max_pt[1], data->max_pt[2],
        data->min_pt[0], data->min_pt[1], data->max_pt[2],

        data->min_pt[0], data->min_pt[1], data->max_pt[2],
        data->max_pt[0], data->min_pt[1], data->max_pt[2],

        data->max_pt[0], data->min_pt[1], data->max_pt[2],
        data->max_pt[0], data->max_pt[1], data->max_pt[2],

        // x-
        data->min_pt[0], data->max_pt[1], data->max_pt[2],
        data->min_pt[0], data->min_pt[1], data->max_pt[2],

        data->min_pt[0], data->min_pt[1], data->max_pt[2],
        data->min_pt[0], data->min_pt[1], data->min_pt[2],

        data->min_pt[0], data->min_pt[1], data->min_pt[2],
        data->min_pt[0], data->max_pt[1], data->min_pt[2],

        data->min_pt[0], data->max_pt[1], data->min_pt[2],
        data->min_pt[0], data->max_pt[1], data->max_pt[2],

        // y-
        data->max_pt[0], data->min_pt[1], data->max_pt[2],
        data->min_pt[0], data->min_pt[1], data->max_pt[2],

        data->min_pt[0], data->min_pt[1], data->max_pt[2],
        data->min_pt[0], data->min_pt[1], data->min_pt[2],

        data->min_pt[0], data->min_pt[1], data->min_pt[2],
        data->max_pt[0], data->min_pt[1], data->min_pt[2],

        data->max_pt[0], data->min_pt[1], data->min_pt[2],
        data->max_pt[0], data->min_pt[1], data->max_pt[2],

        // z-
        data->max_pt[0], data->max_pt[1], data->min_pt[2],
        data->min_pt[0], data->max_pt[1], data->min_pt[2],

        data->min_pt[0], data->max_pt[1], data->min_pt[2],
        data->min_pt[0], data->min_pt[1], data->min_pt[2],

        data->min_pt[0], data->min_pt[1], data->min_pt[2],
        data->max_pt[0], data->min_pt[1], data->min_pt[2],

        data->max_pt[0], data->min_pt[1], data->min_pt[2],
        data->max_pt[0], data->max_pt[1], data->min_pt[2],
    };

    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glCreateBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 144 * sizeof(GLfloat), buf, GL_STATIC_DRAW);
    glLineWidth(1.25f);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);

    mat4 identity;
    glmc_mat4_identity(identity);
    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "model"), 1, GL_FALSE, identity);


    glDrawArrays(GL_LINES, 0, 48);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool SRE_Collider_test_aabbs(sre_aabb *col_1, sre_aabb *col_2)
{
    return
        (col_1->min_pt[0] <= col_2->max_pt[0]) &&
        (col_1->max_pt[0] >= col_2->min_pt[0]) &&
        (col_1->min_pt[1] <= col_2->max_pt[1]) &&
        (col_1->max_pt[1] >= col_2->min_pt[1]) &&
        (col_1->min_pt[2] <= col_2->max_pt[2]) &&
        (col_1->max_pt[2] >= col_2->min_pt[2]);
}

bool SRE_Collider_test_aabb_bsp_tree(sre_aabb *aabb, sre_bsp_node *bsp_tree)
{


    return false;
}

void SRE_Collider_expand(sre_aabb moving_aabb, sre_collider *collider_out)
{
    vec3 dV;
    glmc_vec3_sub(moving_aabb.max_pt, moving_aabb.min_pt, dV);
    glmc_vec3_divs(dV, 2, dV);
    glmc_vec3_add(collider_out->data.aabb.max_pt, dV, collider_out->data.aabb.max_pt);
    glmc_vec3_sub(collider_out->data.aabb.min_pt, dV, collider_out->data.aabb.min_pt);
}

void SRE_Collision_handler_solid(sre_collision_event *event)
{
    vec3 penetration, proj, start;
    glmc_vec3_sub(event->end, event->intersection, penetration);
    float proj_mag = glmc_vec3_dot(penetration, event->normal);
    glmc_vec3_scale(event->normal, proj_mag, proj);
    glmc_vec3_sub(event->end, proj, event->end);
    glmc_vec3_copy(event->intersection, start);

    switch (event->other_collider->type)
    {
        case SRE_COLLIDER_AABB:
        {
            sre_aabb *coldat = &event->other_collider->data.aabb;
            for (size_t i = 0; i < 3; i++)
            {
                event->end[i] = glm_clamp(event->end[i], coldat->min_pt[i] - 0.01f, coldat->max_pt[i] + 0.01f);
            }
            break;
        }
        case SRE_COLLIDER_BSP_TREE:
        {
            bool result = false;
            vec3 old_normal;
            vec3 old_start;
            glmc_vec3_copy(start, old_start);
            glmc_vec3_copy(event->normal, old_normal);
            uint16_t old_front = BSP_EMPTY;
            while (!result)
            {
                sre_bsp_node *root = &event->other_collider->data.bsp_tree;
                uint16_t front = root[event->bsp_tree_parent].children[0];
                if (front == BSP_EMPTY)
                {
                    return;
                }
                if (front == old_front)
                {
                    glmc_vec3_copy(old_start, event->intersection);
                    return;
                }
                old_front = front;
                event->bsp_tree_index = front;
                result = SRE_Line_intersection_bsp_tree(start, event);
            }

            if (result)
            {
                //vec3 normal;
                //glmc_vec3_scale(event->normal, 0.1, normal);
                //glmc_vec3_add(event->intersection, normal, event->end);
                glmc_vec3_copy(event->intersection, event->end);
            }


            break;
        }
    }
}
