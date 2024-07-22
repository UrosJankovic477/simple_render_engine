#include "light.h"
#include "shaders.h"
#include <glad/glad.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void SRE_Light_push_uniform_array(sre_light *light, sre_program program)
{
    char pos_string[64];
    char col_string[64];
    char rad_string[64];

    GLint pos_loc;
    GLint col_loc;
    GLint rad_loc;

    if (light->position[3] != 0.0f)
    {
        light->position[0] /= light->position[3];
        light->position[1] /= light->position[3];
        light->position[2] /= light->position[3];

        sprintf(pos_string, "lights[%d].position", program.light_count);
        pos_loc = glGetUniformLocation(program.id, pos_string);

        sprintf(col_string, "lights[%d].color", program.light_count);
        col_loc = glGetUniformLocation(program.id, col_string);

        sprintf(rad_string, "lights[%d].radius", program.light_count);
        rad_loc = glGetUniformLocation(program.id, rad_string);

        light->idx = program.light_count;
        program.light_count++;
        glProgramUniform1ui(program.id, SRE_Get_uniform_location(&program, "light_count"), program.light_count);
    }
    else
    {
        glm_vec3_normalize(light->position);
        sprintf(pos_string, "inf_lights[%d].position", program.inf_light_count);
        pos_loc = glGetUniformLocation(program.id, pos_string);

        sprintf(col_string, "inf_lights[%d].color", program.inf_light_count);
        col_loc = glGetUniformLocation(program.id, col_string);

        sprintf(rad_string, "inf_lights[%d].radius", program.inf_light_count);
        rad_loc = glGetUniformLocation(program.id, rad_string);

        light->idx = program.inf_light_count;
        program.inf_light_count++;
        glProgramUniform1ui(program.id, SRE_Get_uniform_location(&program, "inf_light_count"), program.inf_light_count);
    }
    
    glProgramUniform3fv(program.id, pos_loc, 1, light->position);
    glProgramUniform1ui(program.id, col_loc, light->color.rgba);
    glProgramUniform1f(program.id, rad_loc, light->radius);
}

void SRE_Light_put_unifrorm_array(sre_light light, sre_program program)
{
    char pos_string[64];
    char col_string[64];
    char rad_string[64];

    GLint pos_loc;
    GLint col_loc;
    GLint rad_loc;

    if (light.position[3] != 0.0f)
    {
        light.position[0] /= light.position[3];
        light.position[1] /= light.position[3];
        light.position[2] /= light.position[3];

        sprintf(pos_string, "lights[%d].position", light.idx);
        pos_loc = glGetUniformLocation(program.id, pos_string);

        sprintf(col_string, "lights[%d].color", light.idx);
        col_loc = glGetUniformLocation(program.id, col_string);

        sprintf(rad_string, "lights[%d].radius", light.idx);
        rad_loc = glGetUniformLocation(program.id, rad_string);
    }
    else
    {
        glm_vec3_normalize(light.position);
        sprintf(pos_string, "inf_lights[%d].position", light.idx);
        pos_loc = glGetUniformLocation(program.id, pos_string);

        sprintf(col_string, "inf_lights[%d].color", light.idx);
        col_loc = glGetUniformLocation(program.id, col_string);

        sprintf(rad_string, "inf_lights[%d].radius", light.idx);
        rad_loc = glGetUniformLocation(program.id, rad_string);
    }
    
    glProgramUniform3fv(program.id, pos_loc, 1, light.position);
    glProgramUniform1ui(program.id, col_loc, light.color.rgba);
    glProgramUniform1f(program.id, rad_loc, light.radius);
}

void SRE_Light_set_position_uniform(sre_light light, sre_program program)
{
    char pos_string[64];
    int pos_loc;

    if (light.position[3] != 0.0f)
    {
        sprintf(pos_string, "lights[%d].position", light.idx);
        pos_loc = glGetUniformLocation(program.id, pos_string);
    }
    else
    {
        sprintf(pos_string, "inf_lights[%d].position", light.idx);
        pos_loc = glGetUniformLocation(program.id, pos_string);
    }
    
    

    glProgramUniform3fv(program.id, pos_loc, 1, light.position);
}

void SRE_Light_set_color_uniform(sre_light light, sre_program program)
{
    char col_string[64];
    int col_loc;

    if (light.position[3] != 0.0f)
    {
        sprintf(col_string, "lights[%d].color", light.idx);
        col_loc = glGetUniformLocation(program.id, col_string);
    }
    else
    {
        sprintf(col_string, "inf_lights[%d].color", light.idx);
        col_loc = glGetUniformLocation(program.id, col_string);
    }
    
    

    glProgramUniform1ui(program.id, col_loc, light.color.rgba);
}

void SRE_Light_set_radius_uniform(sre_light light, sre_program program)
{
    char rad_string[64];

    if (light.position[3] == 0.0f)
    {
        return;
    }
    

    sprintf(rad_string, "lights[%d].radius", light.idx);
    int rad_loc = glGetUniformLocation(program.id, rad_string);

    glProgramUniform1f(program.id, rad_loc, light.radius);
}

void SRE_Debug_draw_light(sre_light light, sre_program program)
{
    GLuint vao = 0;
    GLuint vbo = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glCreateBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, light.position, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    
    glPointSize(5.0);

    glDrawArrays(GL_POINT, 0, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
