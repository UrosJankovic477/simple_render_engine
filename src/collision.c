#include "collision.h"

sre_collider *col_queue[SRE_MAX_NUM_OF_COLS] = {0};
uint8_t col_count = 0;

int SRE_Create_collider(sre_collider *col, void *data, sre_collider_type type)
{
    if (col == NULL || data == NULL ||
        (type & (COL_AABB | COL_CAPSULE | COL_RECT | COL_SPHERE) == 0))
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
    case COL_SPHERE | (COL_SPHERE << 1):
        return SRE_Col_test_spheres(col_1->data, col_2->data);
    case COL_SPHERE | (COL_CAPSULE << 1):
        return SRE_Col_test_capsule_sphere(col_2->data, col_1->data);
    case COL_SPHERE | (COL_AABB << 1):
        return SRE_Col_test_aabb_sphere(col_2->data, col_1->data);
    case COL_CAPSULE | (COL_SPHERE << 1):
        return SRE_Col_test_capsule_sphere(col_1->data, col_2->data);
    case COL_CAPSULE | (COL_CAPSULE << 1):
        return SRE_Col_test_capsules(col_1->data, col_2->data);
    case COL_CAPSULE | (COL_AABB << 1):
        return SRE_Col_test_capsule_aabb(col_1->data, col_2->data);
    case COL_AABB | (COL_SPHERE << 1):
        return SRE_Col_test_aabb_sphere(col_1->data, col_2->data);
    case COL_AABB | (COL_AABB << 1):
        return SRE_Col_test_aabbs(col_1->data, col_2->data);
    case COL_AABB | (COL_CAPSULE << 1):
        return SRE_Col_test_capsule_aabb(col_2->data, col_1->data);
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
    case COL_CAPSULE:
        count = sizeof(sre_coldat_capsule);
        break;
    case COL_RECT:
        count = sizeof(sre_coldat_rect);
        break;
    case COL_SPHERE:
        count = sizeof(sre_coldat_sphere);
        break;
    default:
        return EXIT_FAILURE;
    }
    dest->type = src->type;
    memcpy(dest->data, src->data, count);
    return EXIT_SUCCESS;
}

bool SRE_Col_test_aabb_sphere(sre_coldat_aabb *aabb, sre_coldat_sphere *sphere)
{
    float x = fmax(aabb->min_pt[0], fmin(aabb->max_pt[0], sphere->center[0]));
    float y = fmax(aabb->min_pt[1], fmin(aabb->max_pt[1], sphere->center[1]));
    float z = fmax(aabb->min_pt[2], fmin(aabb->max_pt[2], sphere->center[2]));
    float dist = glm_vec3_distance((vec3){x, y, z}, sphere->center);
    return dist <= sphere->r;
}

bool SRE_Line_intersection(vec3 start, vec3 end, sre_collider *col, vec3 intersection)
{
    uint8_t start_flags, end_flags, intersection_flags;
    sre_coldat_aabb *data = col->data;
    start_flags = (start[0] <= data->min_pt[0]) 
    | ((start[0] >= data->max_pt[0]) << 1)
    | ((start[1] <= data->min_pt[1]) << 2)
    | ((start[1] >= data->max_pt[1]) << 3)
    | ((start[2] <= data->min_pt[2]) << 4)
    | ((start[2] >= data->max_pt[2]) << 5);
    end_flags = (end[0] <= data->min_pt[0]) 
    | ((end[0] >= data->max_pt[0]) << 1)
    | ((end[1] <= data->min_pt[1]) << 2)
    | ((end[1] >= data->max_pt[1]) << 3)
    | ((end[2] <= data->min_pt[2]) << 4)
    | ((end[2] >= data->max_pt[2]) << 5);

    if (!(start_flags | end_flags) || !start_flags)
    {
        // line fully or partially inside collider
        vec3 dv;
        glm_vec3_sub(start, end, dv);
        float t[6];
        float proj[6];
        vec3 start_to_corner[2];
        vec3 normals[6] = {
            {1.0f, 0.0f, 0.0f},
            {-1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, -1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, -1.0f},
        };

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
        glm_vec3_add(start, dvt, intersection);
        if (proj[j] > 0 && end_flags)
        {
            return false;
        }
        vec3 penetration, backtrack;
        glm_vec3_sub(end, intersection, penetration);
        glm_vec3_scale(normals[j], proj[j], backtrack);

        glm_vec3_sub(end, proj, end);
        for (size_t i = 0; i < 3; i++)
        {
            end[i] = glm_clamp(end[i], data->min_pt[i] - 0.01f, data->max_pt[i] + 0.01f);
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
    glm_vec3_sub(start, end, dv);

    float t = 0.0f;

    glm_vec3_copy(start, intersection);
    intersection_flags = start_flags;
    uint8_t hit_dir = -1;
    if (intersection_flags & 0x1)
    {
        // right
        float xmin = data->min_pt[0];
        t = (xmin - start[0]) / dv[0];
        intersection[0] = xmin - 0.01;
        intersection[1] += dv[1] * t;
        intersection[2] += dv[2] * t;
        intersection_flags = ((intersection[1] < data->min_pt[1]) << 2)
        | ((intersection[1] > data->max_pt[1]) << 3)
        | ((intersection[2] < data->min_pt[2]) << 4)
        | ((intersection[2] > data->max_pt[2]) << 5);
        hit_dir = 0;
    }
    else if (intersection_flags & 0x2)
    {
        // left
        float xmax = data->max_pt[0];
        t = (start[0] - xmax) / dv[0];
        intersection[0] = xmax + 0.01;
        intersection[1] += dv[1] * t;
        intersection[2] += dv[2] * t;
        intersection_flags = ((intersection[1] < data->min_pt[1]) << 2)
        | ((intersection[1] > data->max_pt[1]) << 3)
        | ((intersection[2] < data->min_pt[2]) << 4)
        | ((intersection[2] > data->max_pt[2]) << 5);
        hit_dir = 1;
    }
    if (intersection_flags & 0x4)
    {
        // up
        float ymin = data->min_pt[1];
        t = (ymin - intersection[1]) / dv[1];
        intersection[0] += dv[0] * t;
        intersection[1] = ymin - 0.01;
        intersection[2] += dv[2] * t;
        intersection_flags = ((intersection[0] < data->min_pt[0]))
        | ((intersection[0] > data->max_pt[0]) << 1)
        | ((intersection[2] < data->min_pt[2]) << 4)
        | ((intersection[2] > data->max_pt[2]) << 5);
        hit_dir = 2;
    }
    else if (intersection_flags & 0x8)
    {
        // down
        float ymax = data->max_pt[1];
        t = (intersection[1] - ymax) / dv[1];
        intersection[0] += dv[0] * t;
        intersection[1] = ymax + 0.01;
        intersection[2] += dv[2] * t;
        intersection_flags = ((intersection[0] <= data->min_pt[0]))
        | ((intersection[0] >= data->max_pt[0]) << 1)
        | ((intersection[2] <= data->min_pt[2]) << 4)
        | ((intersection[2] >= data->max_pt[2]) << 5);
        hit_dir = 3;
    }
    if (intersection_flags & 0x10)
    {
        // front
        float zmin = data->min_pt[2];
        t = (zmin - intersection[2]) / dv[2];
        intersection[0] += dv[0] * t;
        intersection[1] += dv[1] * t;
        intersection[2] = zmin - 0.01;
        intersection_flags = ((intersection[0] <= data->min_pt[0]))
        | ((intersection[0] >= data->max_pt[0]) << 1)
        | ((intersection[1] <= data->min_pt[1]) << 2)
        | ((intersection[1] >= data->max_pt[1]) << 3);
        hit_dir = 4;
    }
    else if (intersection_flags & 0x20)
    {
        // back
        float zmax = data->max_pt[2];
        t = (intersection[2] - zmax) / dv[2];
        intersection[0] += dv[0] * t;
        intersection[1] += dv[1] * t;
        intersection[2] = zmax + 0.01;
        intersection_flags = ((intersection[0] <= data->min_pt[0]))
        | ((intersection[0] >= data->max_pt[0]) << 1)
        | ((intersection[1] <= data->min_pt[1]) << 2)
        | ((intersection[1] >= data->max_pt[1]) << 3);
        hit_dir = 5;
    }

    vec3 penetration, normal, proj;
    glm_vec3_sub(end, intersection, penetration);
    switch (hit_dir)
    {
    case 0:
        normal[0] = 1;
        normal[1] = 0;
        normal[2] = 0;
        break;
    case 1:
        normal[0] = -1;
        normal[1] = 0;
        normal[2] = 0;
        break;
    case 2:
        normal[0] = 0;
        normal[1] = 1;
        normal[2] = 0;
        break;
    case 3:
        normal[0] = 0;
        normal[1] = -1;
        normal[2] = 0;
        break;
    case 4:
        normal[0] = 0;
        normal[1] = 0;
        normal[2] = 1;
        break;
    case 5:
        normal[0] = 0;
        normal[1] = 0;
        normal[2] = -1;
        break;
    default:
        break;
    }
    float proj_mag = glm_vec3_dot(penetration, normal);
    glm_vec3_scale(normal, proj_mag, proj);
    glm_vec3_sub(end, proj, end);
    for (size_t i = 0; i < 3; i++)
    {
        end[i] = glm_clamp(end[i], data->min_pt[i] - 0.01f, data->max_pt[i] + 0.01f);
    }
    return true;
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
    case COL_CAPSULE:
        SRE_Col_translate_capsule(col->data, xyz);
        break;
    case COL_SPHERE:
        SRE_Col_translate_sphere(col->data, xyz);
        break;
    case COL_AABB:
        SRE_Col_translate_aabb(col->data, xyz);
        break;
    
    default:
        break;
    }
}

void SRE_Col_translate_capsule(sre_coldat_capsule *capsule, vec3 xyz)
{
    capsule->center[0] += xyz[0];
    capsule->center[1] += xyz[2];
    capsule->ymax += xyz[1];
    capsule->ymin += xyz[1];
}

void SRE_Col_translate_sphere(sre_coldat_sphere *sphere, vec3 xyz)
{
    glm_vec3_add(sphere->center, xyz, sphere->center);
}

void SRE_Col_translate_aabb(sre_coldat_aabb *aabb, vec3 xyz)
{
    glm_vec3_add(aabb->min_pt, xyz, aabb->min_pt);
    glm_vec3_add(aabb->max_pt, xyz, aabb->max_pt);
}

bool SRE_Create_mbb(sre_collider *col_out, vec3 *coords, unsigned int n)
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

bool SRE_Col_test_spheres(sre_coldat_sphere *col_1, sre_coldat_sphere *col_2) {
    float dist = glm_vec3_distance(col_1->center, col_2->center);
    return (col_1->r + col_2->r) <= dist;
}

bool SRE_Col_test_capsule_sphere(sre_coldat_capsule *capsule, sre_coldat_sphere *sphere) {
    if (sphere->center[1] > capsule->ymax)
    {
        sre_coldat_sphere upper_sphere;
        upper_sphere.center[0] = capsule->center[0];
        upper_sphere.center[1] = capsule->ymax;
        upper_sphere.center[2] = capsule->center[2];
        upper_sphere.r = capsule->r;
        return SRE_Col_test_spheres(&upper_sphere, sphere);
    }

    if (sphere->center[1] < capsule->ymin)
    {
        sre_coldat_sphere lower_sphere;
        lower_sphere.center[0] = capsule->center[0];
        lower_sphere.center[1] = capsule->ymin;
        lower_sphere.center[2] = capsule->center[2];
        lower_sphere.r = capsule->r;
        return SRE_Col_test_spheres(&lower_sphere, sphere);
    }
    vec2 center_2;
    center_2[0] = sphere->center[0]; 
    center_2[1] = sphere->center[2]; 
    
    float dist = glm_vec2_distance(capsule->center, center_2);
    return (capsule->r  + sphere->r) <= dist;
}

bool SRE_Col_test_capsules(sre_coldat_capsule *col_1, sre_coldat_capsule *col_2) {
    if (col_1->ymin > col_2->ymax)
    {
        sre_coldat_sphere upper_sphere;
        upper_sphere.center[0] = col_2->center[0];
        upper_sphere.center[1] = col_2->ymax;
        upper_sphere.center[2] = col_2->center[2];
        upper_sphere.r = col_2->r;

        sre_coldat_capsule lower_sphere;
        lower_sphere.center[0] = col_1->center[0];
        lower_sphere.center[1] = col_1->ymin;
        lower_sphere.center[2] = col_1->center[2];
        lower_sphere.r = col_1->r;
        return SRE_Col_test_spheres(&upper_sphere, &lower_sphere);
    }

    if (col_2->ymin > col_1->ymax)
    {
        sre_coldat_sphere upper_sphere;
        upper_sphere.center[0] = col_1->center[0];
        upper_sphere.center[1] = col_1->ymax;
        upper_sphere.center[2] = col_1->center[2];
        upper_sphere.r = col_1->r;

        sre_coldat_capsule lower_sphere;
        lower_sphere.center[0] = col_2->center[0];
        lower_sphere.center[1] = col_2->ymin;
        lower_sphere.center[2] = col_2->center[2];
        lower_sphere.r = col_2->r;
        return SRE_Col_test_spheres(&upper_sphere, &lower_sphere);
    }

    float dist = glm_vec2_distance(col_1->center, col_2->center);
    return (col_1->r  + col_2->r) <= dist;
}

bool SRE_Col_test_capsule_aabb(sre_coldat_capsule *capsule, sre_coldat_aabb *aabb) {
    if (aabb->min_pt[1] > capsule->ymax)
    {
        sre_coldat_sphere upper_sphere;
        upper_sphere.center[0] = capsule->center[0];
        upper_sphere.center[1] = capsule->ymax;
        upper_sphere.center[2] = capsule->center[2];
        upper_sphere.r = capsule->r;
        return SRE_Col_test_aabb_sphere(&upper_sphere, aabb);
    }
    
    if (aabb->max_pt[1] < capsule->ymin)
    {
        sre_coldat_sphere lower_sphere;
        lower_sphere.center[0] = capsule->center[0];
        lower_sphere.center[1] = capsule->ymin;
        lower_sphere.center[2] = capsule->center[2];
        lower_sphere.r = capsule->r;
        return SRE_Col_test_aabb_sphere(&lower_sphere, aabb);
    }

    float x = fmax(aabb->min_pt[0], fmin(aabb->max_pt[0], capsule->center[0]));
    float z = fmax(aabb->min_pt[2], fmin(aabb->max_pt[2], capsule->center[1]));
    float dist = glm_vec2_distance((vec2){x, z}, capsule->center);
    return dist <= capsule->r;
}

void SRE_Expand_collision(sre_collider *moving_col, sre_collider *static_col, sre_collider *expanded_col)
{
    // only aabb for now, might add other types of colliders
    expanded_col->type = COL_AABB;
    sre_coldat_aabb *data_expanded = expanded_col->data;
    sre_coldat_aabb *data_static = static_col->data;
    sre_coldat_aabb *data_moving = moving_col->data;
    float dx_2 = (data_moving->max_pt[0] - data_moving->min_pt[0]) / 2;
    float dy_2 = (data_moving->max_pt[1] - data_moving->min_pt[1]) / 2;
    float dz_2 = (data_moving->max_pt[2] - data_moving->min_pt[2]) / 2;
    data_expanded->max_pt[0] = data_static->max_pt[0] + dx_2;
    data_expanded->max_pt[1] = data_static->max_pt[1] + dy_2;
    data_expanded->max_pt[2] = data_static->max_pt[2] + dz_2;
    data_expanded->min_pt[0] = data_static->min_pt[0] - dx_2;
    data_expanded->min_pt[1] = data_static->min_pt[1] - dy_2;
    data_expanded->min_pt[2] = data_static->min_pt[2] - dz_2;
}

// unused for now
/*
void expand_sphere_with_sphere(sre_collider *moving_col, sre_collider *static_col, sre_collider *expanded_col)
{
    expanded_col->type = COL_SPHERE;
    expanded_col->data = malloc(sizeof(sre_coldat_sphere));
    sre_coldat_sphere *coldata_moving = (sre_coldat_sphere*)moving_col->data;
    sre_coldat_sphere *coldata_static = (sre_coldat_sphere*)static_col->data;
    sre_coldat_sphere *coldata_exp = (sre_coldat_sphere*)expanded_col->data;
    glm_vec3_copy(coldata_static->center, coldata_exp->center);
    coldata_exp->r = coldata_static->r + coldata_moving->r;
}

void expand_capsule_with_sphere(sre_collider *moving_col, sre_collider *static_col, sre_collider *expanded_col)
{
    expanded_col->type = COL_CAPSULE;
    expanded_col->data = malloc(sizeof(sre_coldat_capsule));
    sre_coldat_sphere *coldata_moving = (sre_coldat_sphere*)moving_col->data;
    sre_coldat_capsule *coldata_static = (sre_coldat_capsule*)static_col->data;
    sre_coldat_capsule *coldata_exp = (sre_coldat_capsule*)expanded_col->data;
    glm_vec2_copy(coldata_static->center, coldata_exp->center);
    coldata_exp->ymax = coldata_static->ymax + coldata_moving->r;
    coldata_exp->ymin = coldata_static->ymin + coldata_moving->r;
    coldata_exp->r = coldata_static->r + coldata_moving->r;
}
*/

/*
void expand_collision(sre_collider *moving_col, sre_collider *static_col, sre_collider *expanded_col)
{
    
    switch (moving_col->type | (static_col->type << 1))
    {
    case COL_SPHERE | (COL_SPHERE << 1):
        {
            expand_sphere_with_sphere(moving_col, static_col, expanded_col);
            break;
        }
    case COL_SPHERE | (COL_CAPSULE << 1):
        {
            
            break;
        }
    case COL_SPHERE | (COL_AABB << 1):
        {
            expanded_col->type = COL_AABB;
            expanded_col->data = malloc(sizeof(sre_coldat_aabb));
            sre_coldat_sphere *coldata_moving = (sre_coldat_sphere*)moving_col->data;
            sre_coldat_aabb *coldata_static = (sre_coldat_aabb*)static_col->data;
            sre_coldat_aabb *coldata_exp = (sre_coldat_aabb*)expanded_col->data;
            coldata_exp->max_pt[0] = coldata_static->max_pt[0] + coldata_moving->r;
            coldata_exp->max_pt[1] = coldata_static->max_pt[1] + coldata_moving->r;
            coldata_exp->max_pt[2] = coldata_static->max_pt[2] + coldata_moving->r;

            coldata_exp->min_pt[0] = coldata_static->min_pt[0] + coldata_moving->r;
            coldata_exp->min_pt[1] = coldata_static->min_pt[1] + coldata_moving->r;
            coldata_exp->min_pt[2] = coldata_static->min_pt[2] + coldata_moving->r;
            break;
        }
    case COL_CAPSULE | (COL_SPHERE << 1):
        {
            expanded_col->type = COL_CAPSULE;
            expanded_col->data = malloc(sizeof(sre_coldat_capsule));
            sre_coldat_capsule *coldata_moving = (sre_coldat_capsule*)moving_col->data;
            sre_coldat_sphere *coldata_static = (sre_coldat_sphere*)static_col->data;
            sre_coldat_capsule *coldata_exp = (sre_coldat_capsule*)expanded_col->data;
            coldata_exp->center[0] = coldata_static->center[0];
            coldata_exp->center[1] = coldata_static->center[2];
            coldata_exp->ymax = coldata_moving->ymax;
            coldata_exp->ymin = coldata_moving->ymin;
            coldata_exp->r = coldata_moving->r + coldata_static->r;
            break;
        }
    case COL_CAPSULE | (COL_CAPSULE << 1):
        {
            
            break;
        }
    case COL_CAPSULE | (COL_AABB << 1):
        {
            break;
        }
    case COL_AABB | (COL_SPHERE << 1):
        {
            break;
        }
    case COL_AABB | (COL_AABB << 1):
        {
            break;
        }
    case COL_AABB | (COL_CAPSULE << 1):
        {
            break;
        }
    default:
        break;
    }
}
*/



