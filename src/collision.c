#include "collision.h"

sre_collider *col_queue[SRE_MAX_NUM_OF_COLS] = {0};
uint8_t col_count = 0;

enum 
{
    BSP_EMPTY = 0xffff,
    BSP_SOLID = 0xfffe,
};

int SRE_Create_collider(sre_collider *col, void *data, sre_collider_type type)
{
    if (col == NULL || data == NULL ||
        (type & (COL_AABB | COL_BSP_TREE) == 0))
    {
        return EXIT_FAILURE;
    }
    col->col_queue_idx = -1;
    col->data = data;
    col->type = type;
    return EXIT_SUCCESS;
}

bool SRE_Col_test(sre_collider *col_1, sre_collider *col_2)
{
    switch (col_1->type | (col_2->type << 1))
    {
    case COL_AABB | (COL_AABB << 1):
        return SRE_Col_test_aabbs(col_1->data, col_2->data);
    default:
        break;
    }

    return false;
}

int SRE_Copy_collider(sre_collider *src, sre_collider *dest)
{
    size_t count;
    switch (src->type)
    {
    case COL_AABB:
        count = sizeof(sre_coldat_aabb);
        break;
    default:
        return EXIT_FAILURE;
    }
    dest->type = src->type;
    memcpy(dest->data, src->data, count);
    return EXIT_SUCCESS;
}

bool SRE_Line_intersection_bsp_tree(vec3 start, sre_collision_context *context) 
{
    uint16_t index = context->bsp_tree_index;
    if (index == BSP_EMPTY)
    {
        return false;
    }

    if (index == BSP_SOLID)
    {
        glm_vec3_copy(start, context->intersection);
        return true;
    }
    sre_coldat_bsp_tree *root = (sre_coldat_bsp_tree *)context->coldat;
    sre_coldat_bsp_tree node = root[index];
    vec3 normal; 
    glm_vec3_copy(node.deviding_plane, normal);
    float d = node.deviding_plane[3];
    vec3 dx;
    glm_vec3_sub(context->end, start, dx);
    float t1 = glm_dot(start, normal);
    float t2 = glm_dot(context->end, normal);

    uint8_t start_side = t1 <= d;
    uint8_t end_side = t2 <= d;

    
    if (start_side == end_side)
    {
        context->bsp_tree_index = node.children[start_side];
        return SRE_Line_intersection_bsp_tree(start, context);
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
    
    

    glm_vec3_scale(dx, t, mid);
    glm_vec3_add(mid, start, mid);

    glm_vec3_copy(normal, context->normal);
    context->bsp_tree_parent = index;

    context->bsp_tree_index = node.children[start_side];
    vec3 old_end;
    glm_vec3_copy(context->end, old_end);
    glm_vec3_copy(mid, context->end);
    bool result = SRE_Line_intersection_bsp_tree(start, context);
    if (result)
    {
        return result;
    }
    glm_vec3_copy(old_end, context->end);
    context->bsp_tree_index = node.children[end_side];
    return SRE_Line_intersection_bsp_tree(mid, context);
}

bool SRE_Line_intersection_aabb(vec3 start, sre_collision_context *context)
{
    uint8_t start_flags, end_flags, intersection_flags;
    sre_coldat_aabb *data = (sre_coldat_aabb *)context->coldat;
    start_flags = (start[0] <= data->min_pt[0]) 
    | ((start[0] >= data->max_pt[0]) << 1)
    | ((start[1] <= data->min_pt[1]) << 2)
    | ((start[1] >= data->max_pt[1]) << 3)
    | ((start[2] <= data->min_pt[2]) << 4)
    | ((start[2] >= data->max_pt[2]) << 5);
    end_flags = (context->end[0] <= data->min_pt[0]) 
    | ((context->end[0] >= data->max_pt[0]) << 1)
    | ((context->end[1] <= data->min_pt[1]) << 2)
    | ((context->end[1] >= data->max_pt[1]) << 3)
    | ((context->end[2] <= data->min_pt[2]) << 4)
    | ((context->end[2] >= data->max_pt[2]) << 5);

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
        glm_vec3_sub(start, context->end, dv);
        float t[6];
        float proj[6];
        vec3 start_to_corner[2];

        glm_vec3_sub(start, data->min_pt, start_to_corner[0]);
        glm_vec3_sub(start, data->max_pt, start_to_corner[1]);

        float closest = 1.0f;
        uint8_t j;

        for (uint8_t i = 0; i < 6; i++)
        {
            uint8_t idx = i & 1;
            proj[i] = glm_vec3_dot(normals[i], dv);
            t[i] = glm_vec3_dot(normals[i], start_to_corner[idx]) / -proj[i];
            if (fabsf(t[i]) < fabsf(closest))
            {
                closest = t[i];
                j = i;
            }
        }
        vec3 dvt;
        glm_vec3_scale(dv, closest, dvt);
        glm_vec3_add(start, dvt, context->intersection);
        if (proj[j] > 0 && end_flags)
        {
            return false;
        }
        vec3 penetration, backtrack;
        glm_vec3_sub(context->end, context->intersection, penetration);
        glm_vec3_scale(normals[j], proj[j], backtrack);

        glm_vec3_sub(context->end, proj, context->end);
        for (size_t i = 0; i < 3; i++)
        {
            context->end[i] = glm_clamp(context->end[i], data->min_pt[i] - 0.01f, data->max_pt[i] + 0.01f);
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
    glm_vec3_sub(start, context->end, dv);

    float t = 0.0f;

    glm_vec3_copy(start, context->intersection);
    intersection_flags = start_flags;
    uint8_t hit_dir = -1;
    if (intersection_flags & 0x1)
    {
        // right
        float xmin = data->min_pt[0];
        t = (xmin - start[0]) / dv[0];
        context->intersection[0] = xmin - 0.01;
        context->intersection[1] += dv[1] * t;
        context->intersection[2] += dv[2] * t;
        intersection_flags = ((context->intersection[1] < data->min_pt[1]) << 2)
        | ((context->intersection[1] > data->max_pt[1]) << 3)
        | ((context->intersection[2] < data->min_pt[2]) << 4)
        | ((context->intersection[2] > data->max_pt[2]) << 5);
        hit_dir = 0;
    }
    else if (intersection_flags & 0x2)
    {
        // left
        float xmax = data->max_pt[0];
        t = (start[0] - xmax) / dv[0];
        context->intersection[0] = xmax + 0.01;
        context->intersection[1] += dv[1] * t;
        context->intersection[2] += dv[2] * t;
        intersection_flags = ((context->intersection[1] < data->min_pt[1]) << 2)
        | ((context->intersection[1] > data->max_pt[1]) << 3)
        | ((context->intersection[2] < data->min_pt[2]) << 4)
        | ((context->intersection[2] > data->max_pt[2]) << 5);
        hit_dir = 1;
    }
    if (intersection_flags & 0x4)
    {
        // up
        float ymin = data->min_pt[1];
        t = (ymin - context->intersection[1]) / dv[1];
        context->intersection[0] += dv[0] * t;
        context->intersection[1] = ymin - 0.01;
        context->intersection[2] += dv[2] * t;
        intersection_flags = ((context->intersection[0] < data->min_pt[0]))
        | ((context->intersection[0] > data->max_pt[0]) << 1)
        | ((context->intersection[2] < data->min_pt[2]) << 4)
        | ((context->intersection[2] > data->max_pt[2]) << 5);
        hit_dir = 2;
    }
    else if (intersection_flags & 0x8)
    {
        // down
        float ymax = data->max_pt[1];
        t = (context->intersection[1] - ymax) / dv[1];
        context->intersection[0] += dv[0] * t;
        context->intersection[1] = ymax + 0.01;
        context->intersection[2] += dv[2] * t;
        intersection_flags = ((context->intersection[0] <= data->min_pt[0]))
        | ((context->intersection[0] >= data->max_pt[0]) << 1)
        | ((context->intersection[2] <= data->min_pt[2]) << 4)
        | ((context->intersection[2] >= data->max_pt[2]) << 5);
        hit_dir = 3;
    }
    if (intersection_flags & 0x10)
    {
        // front
        float zmin = data->min_pt[2];
        t = (zmin - context->intersection[2]) / dv[2];
        context->intersection[0] += dv[0] * t;
        context->intersection[1] += dv[1] * t;
        context->intersection[2] = zmin - 0.01;
        intersection_flags = ((context->intersection[0] <= data->min_pt[0]))
        | ((context->intersection[0] >= data->max_pt[0]) << 1)
        | ((context->intersection[1] <= data->min_pt[1]) << 2)
        | ((context->intersection[1] >= data->max_pt[1]) << 3);
        hit_dir = 4;
    }
    else if (intersection_flags & 0x20)
    {
        // back
        float zmax = data->max_pt[2];
        t = (context->intersection[2] - zmax) / dv[2];
        context->intersection[0] += dv[0] * t;
        context->intersection[1] += dv[1] * t;
        context->intersection[2] = zmax + 0.01;
        intersection_flags = ((context->intersection[0] <= data->min_pt[0]))
        | ((context->intersection[0] >= data->max_pt[0]) << 1)
        | ((context->intersection[1] <= data->min_pt[1]) << 2)
        | ((context->intersection[1] >= data->max_pt[1]) << 3);
        hit_dir = 5;
    }

    glm_vec3_copy(normals[hit_dir], context->normal);

    return true;
}

bool SRE_Line_intersection(vec3 start, sre_collision_context *context, void (*col_handler)(sre_collision_context *, void *))
{
    vec3 plane_normal;
    bool intersects;
    if (context->type == COL_BSP_TREE)
    {
        intersects = SRE_Line_intersection_bsp_tree(start, context);
    }
    else if (context->type == COL_AABB)
    {
        sre_coldat_aabb expanded_col;
        sre_coldat_aabb col_data;
        SRE_Expand_collision(context->moving_collider, context->coldat, &expanded_col);
        intersects = SRE_Line_intersection_aabb(start, context);
    }
    if (intersects)
    {
        col_handler(context, NULL);
    }
    
    return intersects;
}

bool SRE_Col_load(sre_collider *col)
{
    if (col_count >= SRE_MAX_NUM_OF_COLS)
    {
        return false;
    }
    col->col_queue_idx = col_count;
    col_queue[col_count++] = col;
    return true;
}

void SRE_Col_unload(sre_collider *col)
{
    memmove_s(col_queue + col->col_queue_idx, SRE_MAX_NUM_OF_COLS - col->col_queue_idx, col_queue + col->col_queue_idx + 1, col_count - col->col_queue_idx);
    col_count--;
}

void SRE_Col_translate(sre_collider *col, vec3 xyz)
{
    switch (col->type)
    {
    case COL_AABB:
        SRE_Col_translate_aabb(col->data, xyz);
        break;
    
    default:
        break;
    }
}

void SRE_Col_translate_aabb(sre_coldat_aabb *aabb, vec3 xyz)
{
    glm_vec3_add(aabb->min_pt, xyz, aabb->min_pt);
    glm_vec3_add(aabb->max_pt, xyz, aabb->max_pt);
}

bool SRE_Create_mesh_boudning_box(sre_collider *col_out, vec3 *coords, unsigned int n)
{
    col_out->type = COL_AABB;
    if (col_out->data == NULL)
    {
        // no collision data ptr was bound;
        return false;
    }
    sre_coldat_aabb *aabb_out = (sre_coldat_aabb*)col_out->data;
    glm_vec3_copy(coords[0], aabb_out->min_pt);
    glm_vec3_copy(coords[0], aabb_out->max_pt);
    for (size_t i = 0; i < n; i++)
    {
        if (coords[i][0] > aabb_out->max_pt[0])
        {
            aabb_out->max_pt[0] = coords[i][0];
        }
        if (coords[i][1] > aabb_out->max_pt[1])
        {
            aabb_out->max_pt[1] = coords[i][1];
        }
        if (coords[i][2] > aabb_out->max_pt[2])
        {
            aabb_out->max_pt[2] = coords[i][2];
        } 
        if (coords[i][0] < aabb_out->min_pt[0])
        {
            aabb_out->min_pt[0] = coords[i][0];
        }
        if (coords[i][1] < aabb_out->min_pt[1])
        {
            aabb_out->min_pt[1] = coords[i][1];
        }
        if (coords[i][2] < aabb_out->min_pt[2])
        {
            aabb_out->min_pt[2] = coords[i][2];
        }
    }
    return true;
}

void SRE_Col_get_center(sre_collider *col, vec3 center)
{
    sre_coldat_aabb *data = (sre_coldat_aabb*)col->data;
    glm_vec3_add(data->max_pt, data->min_pt, center);
    glm_vec3_scale(center, 0.5, center);
}

void SRE_Draw_collider(sre_collider *col, sre_program program)
{
    GLint vao;
    GLint vbo;
    sre_coldat_aabb *data = col->data;
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
    glm_mat4_identity(identity);
    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "model"), 1, GL_FALSE, identity);


    glDrawArrays(GL_LINES, 0, 48);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool SRE_Col_test_aabbs(sre_coldat_aabb *col_1, sre_coldat_aabb *col_2)
{
    return
        (col_1->min_pt[0] <= col_2->max_pt[0]) &&
        (col_1->max_pt[0] >= col_2->min_pt[0]) &&
        (col_1->min_pt[1] <= col_2->max_pt[1]) &&
        (col_1->max_pt[1] >= col_2->min_pt[1]) &&
        (col_1->min_pt[2] <= col_2->max_pt[2]) &&
        (col_1->max_pt[2] >= col_2->min_pt[2]);
}

bool SRE_Col_test_aabb_bsp_tree(sre_coldat_aabb *aabb, sre_coldat_bsp_tree *bsp_tree)
{


    return false;
}

void SRE_Expand_collision(sre_coldat_aabb *coldat_moving, sre_coldat_aabb *coldat_static, sre_coldat_aabb *coldat_expanded)
{
    float dx_2 = (coldat_moving->max_pt[0] - coldat_moving->min_pt[0]) / 2;
    float dy_2 = (coldat_moving->max_pt[1] - coldat_moving->min_pt[1]) / 2;
    float dz_2 = (coldat_moving->max_pt[2] - coldat_moving->min_pt[2]) / 2;
    coldat_expanded->max_pt[0] = coldat_static->max_pt[0] + dx_2;
    coldat_expanded->max_pt[1] = coldat_static->max_pt[1] + dy_2;
    coldat_expanded->max_pt[2] = coldat_static->max_pt[2] + dz_2;
    coldat_expanded->min_pt[0] = coldat_static->min_pt[0] - dx_2;
    coldat_expanded->min_pt[1] = coldat_static->min_pt[1] - dy_2;
    coldat_expanded->min_pt[2] = coldat_static->min_pt[2] - dz_2;
}

void SRE_Col_handler_solid(sre_collision_context *context, void *args)
{
    vec3 penetration, proj, start;
    glm_vec3_sub(context->end, context->intersection, penetration);
    float proj_mag = glm_vec3_dot(penetration, context->normal);
    glm_vec3_scale(context->normal, proj_mag, proj);
    glm_vec3_sub(context->end, proj, context->end);
    glm_vec3_copy(context->intersection, start);
    
    switch (context->type)
    {
        case COL_AABB:
        {
            sre_coldat_aabb *coldat = (sre_coldat_aabb *)context->coldat;
            for (size_t i = 0; i < 3; i++)
            {
                context->end[i] = glm_clamp(context->end[i], coldat->min_pt[i] - 0.01f, coldat->max_pt[i] + 0.01f);
            }
            break;
        }
        case COL_BSP_TREE:
        { 
            bool result = false;
            vec3 old_normal;
            vec3 old_start;
            glm_vec3_copy(start, old_start);
            glm_vec3_copy(context->normal, old_normal);
            uint16_t old_front = BSP_EMPTY;
            while (!result)
            {
                sre_coldat_bsp_tree *root = (sre_coldat_bsp_tree *)context->coldat;
                uint16_t front = root[context->bsp_tree_parent].children[0];
                if (front == BSP_EMPTY)
                {
                    return;
                }
                if (front == old_front)
                {
                    glm_vec3_copy(old_start, context->intersection);
                    return;
                }
                old_front = front;
                context->bsp_tree_index = front;
                result = SRE_Line_intersection_bsp_tree(start, context);
            }

            if (result)
            {
                //vec3 normal;
                //glm_vec3_scale(context->normal, 0.1, normal);
                //glm_vec3_add(context->intersection, normal, context->end);
                glm_vec3_copy(context->intersection, context->end);
            }
            

            break;
        }
    }
}
