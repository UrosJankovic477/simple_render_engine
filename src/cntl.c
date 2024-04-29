#include "cntl.h"

void SRE_Create_cntl(sre_cntl_handle *cntl, float yaw, float pitch, float speed, float sensitivity, vec3 orig, vec3 up)
{
    cntl->flags = 0;
    cntl->yaw = yaw;
    cntl->pitch = pitch;
    cntl->speed = speed;
    cntl->sensitivity = sensitivity;
    glm_vec3_copy(orig, cntl->orig);
    glm_vec3_copy(up, cntl->up);

    float cosx, cosy, sinx, siny;
    cosx = cosf(cntl->pitch);
    sinx = sinf(cntl->pitch);
    cosy = cosf(cntl->yaw);
    siny = sinf(cntl->yaw);

    cntl->right[0] = sinx;
    cntl->right[1] = 0;
    cntl->right[2] = -cosx;
    cntl->dir[0] = cosx * cosy;
    cntl->dir[1] = siny;
    cntl->dir[2] = sinx * cosy;
}

void SRE_Create_cntl_indirect(sre_cntl_handle *src_cntl, sre_cntl_handle *dest_cntl)
{
    SRE_Create_cntl(dest_cntl, src_cntl->yaw, src_cntl->pitch, src_cntl->speed, src_cntl->sensitivity, src_cntl->orig, src_cntl->up);
}

void SRE_Cntl_translate(sre_cntl_handle *cntl, float dt)
{
    float dist = cntl->speed * dt;
    vec3 dist_dir;
    vec3 dist_right;
    glm_vec3_scale(cntl->dir, dist, dist_dir);
    glm_vec3_scale(cntl->right, dist, dist_right);

    if (cntl->flags & CTL_DIR_UP && (cntl->flags ^ CTL_DIR_DOWN) & CTL_DIR_DOWN)
    {
        cntl->orig[1] += dist;
    }
    else if (cntl->flags & CTL_DIR_DOWN && (cntl->flags ^ CTL_DIR_UP) & CTL_DIR_UP)
    {
        cntl->orig[1] -= dist;
    }

    if (cntl->flags & CTL_DIR_RIGHT && (cntl->flags ^ CTL_DIR_LEFT) & CTL_DIR_LEFT)
    {
        glm_vec3_add(cntl->orig, dist_right, cntl->orig);
    }
    else if (cntl->flags & CTL_DIR_LEFT && (cntl->flags ^ CTL_DIR_RIGHT) & CTL_DIR_RIGHT)
    {
        glm_vec3_sub(cntl->orig, dist_right, cntl->orig);
    }

    if (cntl->flags & CTL_DIR_BACK && (cntl->flags ^ CTL_DIR_FRONT) & CTL_DIR_FRONT)
    {
        glm_vec3_sub(cntl->orig, dist_dir, cntl->orig);
    }
    else if (cntl->flags & CTL_DIR_FRONT && (cntl->flags ^ CTL_DIR_BACK) & CTL_DIR_BACK)
    {
        glm_vec3_add(cntl->orig, dist_dir, cntl->orig);
    }
}

void SRE_Cntl_rotate(sre_cntl_handle *cntl, float dt, float dx, float dy)
{
    if (cntl->flags & CTL_MOUSE_MOVEMENT)
    {
        dx *= (float)dt * cntl->sensitivity;
        dy *= -(float)dt * cntl->sensitivity;

        cntl->yaw = fmodf(cntl->yaw + dx, M_PI * 2);
        cntl->pitch = glm_clamp(cntl->pitch + dy, -M_PI_2 + 0.1f, M_PI_2 - 0.1f);

        float cosx, cosy, sinx, siny;
        cosx = cosf(cntl->yaw);
        sinx = sinf(cntl->yaw);
        cosy = cosf(cntl->pitch);
        siny = sinf(cntl->pitch);

        cntl->right[0] = -sinx;
        cntl->right[2] = cosx;

        cntl->dir[0] = cosx * cosy;
        cntl->dir[1] = siny;
        cntl->dir[2] = sinx * cosy;
    }
}
