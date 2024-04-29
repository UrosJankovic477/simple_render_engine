#include "shaders.h"


int SRE_Compile_shader(const char *p_shader_path, GLenum p_shader_type, GLuint *p_shader_obj)
{
    FILE *shader_file = fopen(p_shader_path, "r");
    if (!shader_file)
    {
        fprintf(stderr, "Failed to open file at location: %s\n", p_shader_path);
        return GL_SHADER_COMPILED_FALSE;
    }

    char *buffer = NULL;
    GLint size = 0;
    size_t bytes_read;
    while (!feof(shader_file))
    {
        buffer = realloc(buffer, (size + 100)*sizeof(char));
        bytes_read = fread(buffer + size, sizeof(char), 100, shader_file);
        size += bytes_read;
    }
    buffer[size] = '\0';
    

    *p_shader_obj = glCreateShader(p_shader_type);
    glShaderSource(*p_shader_obj, 1, (const char* const*)&buffer, (const GLint *)&size);
    glCompileShader(*p_shader_obj);

    GLint shader_compiled;
    glGetShaderiv(*p_shader_obj, GL_COMPILE_STATUS, &shader_compiled);
    if (shader_compiled == GL_FALSE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(*p_shader_obj, 1024, &log_length, message);
        fprintf(stderr, "failed to compile shader.\nLog: %s\nShader code:\n%s\n", message, buffer);
        return GL_SHADER_COMPILED_FALSE;
    }
    return GL_SHADER_COMPILED_TRUE;
}

int SRE_Create_shader_program(sre_program *program, GLuint vertex_shader, GLuint fragment_shader)
{
    program->light_count = 0;
    program->inf_light_count = 0;

    program->uniforms_count = 0;
    program->id = glCreateProgram();
    
    for (size_t i = 0; i < SH_MAX_UNIFORM_NUM; i++)
    {
        program->uniform_cache[i].location = -1;
    }
    
    program->vertex_shader = vertex_shader;
    program->fragment_shader = fragment_shader;

    glAttachShader(program->id, vertex_shader);
    glAttachShader(program->id, fragment_shader);
    glLinkProgram(program->id);

    GLint program_linked = 0;
    glGetProgramiv(program->id, GL_LINK_STATUS, (int*)&program_linked);
    if (program_linked == GL_FALSE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program->id, 1024, &log_length, message);
        fprintf(stderr, "failed to link program.\nLog: %s\n", message);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int SRE_Get_uniform_location(sre_program *program, const char *uniform_name)
{
    uint8_t hash = get_string_hash8(uniform_name);
    sre_uniform uniform = program->uniform_cache[hash];
    if (uniform.location != -1)
    {
        uint8_t i = 1;
        while (uniform.location != -1 && strcmp(uniform.name, uniform_name) != 0)
        {
            hash += i;
            uniform = program->uniform_cache[hash];
            i++;
            if (i == SH_MAX_UNIFORM_NUM)
            {
                return -1;
            }
        }
    }
    program->uniform_cache[hash].location = glGetUniformLocation(program->id, uniform_name); 
    program->uniform_cache[hash].name = uniform_name;
    uniform = program->uniform_cache[hash];
    program->uniforms_count++;

    return uniform.location;
}

void SRE_Delete_shader_program(sre_program *program)
{
    glDeleteProgram(program->id);
}