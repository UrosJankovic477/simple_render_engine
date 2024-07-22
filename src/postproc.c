#include "postproc.h"

float postproc_plane[] = {
    -1.0f, -1.0f, 0.0f  ,  0.0f, 0.0f,
    1.0f, -1.0f, 0.0f   ,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f  ,  0.0f, 1.0f,
    -1.0f,  1.0f, 0.0f  ,  0.0f, 1.0f,
    1.0f, -1.0f, 0.0f   ,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f   ,  1.0f, 1.0f,
};

GLuint postproc_plane_vao = 0;
GLuint postproc_plane_vbo = 0;
GLuint postproc_program = 0;
GLuint postproc_vert_shdr = 0;
GLuint postproc_frag_shdr = 0;
GLint postproc_uniform_tex_id = 0;
GLint postproc_uniform_time = 0;

void SRE_Init_postproc()
{
    glGenVertexArrays(1, &postproc_plane_vao);
    glBindVertexArray(postproc_plane_vao);

    glGenBuffers(1, &postproc_plane_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, postproc_plane_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(postproc_plane), postproc_plane, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    int compiled_veretex_shader = SRE_Compile_shader("../src/shaders/postproc_vert_shdr.glsl", GL_VERTEX_SHADER, &postproc_vert_shdr);
    if (compiled_veretex_shader == GL_SHADER_COMPILED_FALSE)
    {
        exit(EXIT_FAILURE);
    }
    
    int compiled_fragmet_shader = SRE_Compile_shader("../src/shaders/postproc_frag_shdr.glsl", GL_FRAGMENT_SHADER, &postproc_frag_shdr);
    if (compiled_fragmet_shader == GL_SHADER_COMPILED_FALSE)
    {
        exit(EXIT_FAILURE);
    }

    postproc_program = glCreateProgram();
    glAttachShader(postproc_program, postproc_vert_shdr);
    glAttachShader(postproc_program, postproc_frag_shdr);
    glLinkProgram(postproc_program);


    GLint program_linked = 0;
    glGetProgramiv(postproc_program, GL_LINK_STATUS, (int*)&program_linked);
    if (program_linked == GL_FALSE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(postproc_program, 1024, &log_length, message);
        fprintf(stderr, "failed to link program.\nLog: %s\n", message);
        exit(EXIT_FAILURE);
    }

    glUseProgram(postproc_program);

    postproc_uniform_tex_id = glGetUniformLocation(postproc_program, "rendered_texture");
    postproc_uniform_time = glGetUniformLocation(postproc_program, "time");

    glUseProgram(0);

}

void SRE_Delete_postproc_plane()
{
    glDeleteBuffers(1, &postproc_plane_vbo);
    glDeleteVertexArrays(1, &postproc_plane_vao);
}

int SRE_Framebuffer_init(sre_framebuffer *framebuffer, GLsizei width, GLsizei height, GLenum fmt)
{
    framebuffer->width = width;
    framebuffer->height = height;

    glGenTextures(2, framebuffer->textures);

    glBindTexture(GL_TEXTURE_2D, framebuffer->maps.color_map);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, fmt, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


    glBindTexture(GL_TEXTURE_2D, framebuffer->maps.depth_map);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


    glGenFramebuffers(1, &framebuffer->id);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->maps.color_map, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer->maps.depth_map, 0);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void SRE_Framebuffer_bind(sre_framebuffer framebuffer, GLuint program)
{
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, framebuffer.width, framebuffer.height);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
    glUseProgram(program);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SRE_Draw_to_main_framebuffer(sre_framebuffer framebuffer, GLuint postproc_prog, GLsizei width, GLsizei height)
{
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(postproc_prog);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(postproc_plane_vao);
    glBindBuffer(GL_ARRAY_BUFFER, postproc_plane_vbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer.maps.color_map);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void SRE_Delete_framebuffer(sre_framebuffer *framebuffer)
{
    glDeleteTextures(2, framebuffer->textures);
    glDeleteFramebuffers(1, &(framebuffer->id));
}
