#include <time.h>
#include "terrain_generation.h"


int generate_terrain_vertices(uint32_t w, uint32_t h, sre_terrain *out_terrain, unsigned int tex_rep)
{
    uint32_t size = w * h;
    out_terrain->vertex_data = malloc(size * sizeof(sre_vtx_float_float_2_10_10_10));
    out_terrain->indicies = malloc(w * 64);
    out_terrain->basevertex = malloc(w * 32);
    out_terrain->w = w;
    out_terrain->h = h;
    sre_2_10_10_10s up = SRE_Float_to_2_10_10_10s(0, 1, 0);
    unsigned int stride = sizeof(sre_vtx_float_float_2_10_10_10);

    srand(time(NULL));

    //generate indicies;

    for (uint32_t i = 0; i < w; i++)
    {
        out_terrain->indicies[i << 1] = i;
        out_terrain->indicies[(i << 1) | 1] = i + w;
    }
    
    //
    
    for (uint32_t i = 0; i < h; i++)
    {
        sre_vtx_float_float_2_10_10_10 vtx;
        
        vtx.posz = (float)i - (float)(h >> 1); 
        
        vtx.normal = up;

        vtx.v = ((float)i) / ((float)(h) / (float)tex_rep);

        // generate basevertex array

        out_terrain->basevertex[i] = w * i;

        //
        
        for (uint32_t j = 0; j < w; j++)
        {
            vtx.posx = (float)j - (float)(w >> 1);

            vtx.posy = (float)rand() / (float)0xffff;

            vtx.u = (float)j / ((float)(w) / (float)tex_rep);

            out_terrain->vertex_data[j + h * i] = vtx;
        }
        
    }


    glGenVertexArrays(1, &out_terrain->vao);
    glBindVertexArray(out_terrain->vao);

    glGenBuffers(1, &out_terrain->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, out_terrain->vbo);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(sre_vertex), out_terrain->vertex_data, GL_STATIC_DRAW);

    
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 4, GL_INT_2_10_10_10_REV, GL_TRUE, stride, (GLvoid*)(3 * sizeof(float) + 2 * sizeof(float)));
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    
    
    return EXIT_SUCCESS;
}

void draw_terrain(sre_terrain terrain_data, sre_program program)
{
    uint32_t num_of_strips = terrain_data.h - 1;
    GLsizei *count = malloc((terrain_data.w  << 1) * sizeof(GLsizei));
    GLuint **indices = malloc(num_of_strips * sizeof(terrain_data.indicies));
    for (size_t i = 0; i < num_of_strips; i++)
    {
        count[i] = terrain_data.w << 1;
        indices[i] = terrain_data.indicies;
    }

    glBindVertexArray(terrain_data.vao);
    glBindBuffer(GL_ARRAY_BUFFER, terrain_data.vbo);

    mat4 I;
    glm_mat4_identity(I);
    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "model"), 1, GL_FALSE, I);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glMultiDrawElementsBaseVertex(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, (const GLvoid**)indices, num_of_strips, terrain_data.basevertex);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    free(count);
    free(indices);
}

void delete_terrain_vertices(sre_terrain *terrain_data)
{
    free(terrain_data->indicies);
    free(terrain_data->basevertex);
    free(terrain_data->vertex_data);
    glDeleteBuffers(1, &terrain_data->vbo);
    glDeleteVertexArrays(1, &terrain_data->vao);
}
