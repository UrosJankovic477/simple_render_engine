#include <glad/glad.h>
#include "parse_obj.h"
#include "model.h"
#include <SDL2/SDL.h>
#include <cglm/types.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include "camera.h"
#include "light.h"
#include "terrain_generation.h"
#include "postproc.h"
#include "shaders.h"
#include "props.h"
#include "errors.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define DEG2RAD(x) M_PI / 180 * (x)

int main(int argc, char **argv)
{
    // SRE_Init

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    int width = 480 * 2;
    int height = 360 * 2;
    SDL_Window *window = 
    SDL_CreateWindow(   "TEST-APPLICATION",
                        SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED,
                        width, height,
                        SDL_WINDOW_OPENGL);
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    SDL_GL_MakeCurrent(window, glcontext);

    //glEnable (GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    SDL_SetWindowMouseGrab(window, SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    stbi_set_flip_vertically_on_load(1);
    
    sre_framebuffer postproc_framebuffer;
    framebuffer_init(&postproc_framebuffer, 240, 180, GL_RGB);

    GLuint vertex_unlit_shader;
    GLuint fragment_unlit_shader;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint alpha_clip_shader;
    int compiled_veretex_shader = SRE_Compile_shader("../src/shaders/vertex.glsl", GL_VERTEX_SHADER, &vertex_shader);
    if (compiled_veretex_shader == GL_SHADER_COMPILED_FALSE)
    {
        exit(EXIT_FAILURE);
    }
    int compiled_fragment_shader = SRE_Compile_shader("../src/shaders/fragment.glsl", GL_FRAGMENT_SHADER, &fragment_shader);
    if (compiled_fragment_shader == GL_SHADER_COMPILED_FALSE)
    {
        exit(EXIT_FAILURE);
    }
    //int compiled_veretex_unlit_shader = compile_shader("../src/shaders/vertex_unlit.glsl", GL_VERTEX_SHADER, &vertex_unlit_shader);
    //if (compiled_veretex_unlit_shader == GL_SHADER_COMPILED_FALSE)
    //{
    //    exit(EXIT_FAILURE);
    //}
    //int compiled_fragment_unlit_shader = compile_shader("../src/shaders/fragment_unlit.glsl", GL_FRAGMENT_SHADER, &fragment_unlit_shader);
    //if (compiled_fragment_unlit_shader == GL_SHADER_COMPILED_FALSE)
    //{
    //    exit(EXIT_FAILURE);
    //}
    int compiled_alpha_clip_fragment_shader = SRE_Compile_shader("../src/shaders/alpha_clip_fragment.glsl", GL_FRAGMENT_SHADER, &alpha_clip_shader);
    if (compiled_alpha_clip_fragment_shader == GL_SHADER_COMPILED_FALSE)
    {
        exit(EXIT_FAILURE);
    }


    sre_program program;
    sre_program program_alpha_clip;
    sre_program program_unlit;

    SRE_Create_shader_program(&program, vertex_shader, fragment_shader);
    SRE_Create_shader_program(&program_alpha_clip, vertex_shader, alpha_clip_shader);
    //create_shader_program(&program_unlit, vertex_unlit_shader, fragment_unlit_shader);

    init_postproc();

    glUseProgram(program.id);

    sre_camera cam;
    sre_cntl_handle cam_cntl;
    SRE_Create_cntl(&cam_cntl, 0, 0, 10, 20, (vec3){0, 0, 0}, (vec3){0, 1, 0});
    SRE_Cam_init(&cam, &cam_cntl, 0.01f, 200.0f, ((float)width / (float)height), 90.0f);
    

    sre_model_obj kms_model;
    sre_model_obj brick_block_model;
    //sre_model_obj *alpha_clip_test = (sre_model_obj*)malloc(sizeof(sre_model_obj));
    //gl_mesh *alpha_clip_test_mesh = (gl_mesh*)malloc(sizeof(gl_mesh));
    gl_mesh brick_mesh;
    gl_mesh kms_mesh;

    init_parser();
    process_obj_file("../resources/models/kms.obj", &kms_model);
    process_obj_file("../resources/models/brick_cube.obj", &brick_block_model);

    SRE_Load_gl_mesh(brick_block_model.meshes[0], &brick_mesh);
    SRE_Load_gl_mesh(kms_model.meshes[0], &kms_mesh);

    sre_prop block1;
    sre_prop block2;

    mat4 block1_model;
    mat4 block2_model;

    sre_collider block1_col;
    sre_collider block2_col;
    sre_collider kms_col;

    sre_prop kms_uwu;
    
    sre_coldat_aabb block1_col_data;
    sre_coldat_aabb block2_col_data;
    sre_coldat_aabb kms_col_data;

    sre_cntl_handle kms_uwu_cntl;
    SRE_Create_cntl_indirect(cam.cntl, &kms_uwu_cntl);
    glm_vec3_copy((vec3){8, 1, 0}, cam.cntl->orig);
    SRE_Cam_set_view_mat(&cam);
    SRE_Cam_set_proj_mat(&cam);

    block1_col.data = &block1_col_data;
    block2_col.data = &block2_col_data;

    kms_col.data = &kms_col_data;
    SRE_Create_mbb(&block1_col, brick_block_model.meshes[0]->position, brick_block_model.meshes[0]->position_count);
    SRE_Copy_collider(&block1_col, &block2_col);

    SRE_Create_mbb(&kms_col, kms_model.meshes[0]->position, kms_model.meshes[0]->position_count);
    create_prop(&kms_mesh, &kms_col, NULL, &kms_uwu, &kms_uwu_cntl);
    translate_prop(&kms_uwu, (vec3){8, 1, 0});
    create_prop(&brick_mesh, &block1_col, NULL, &block1, NULL);
    translate_prop(&block1, (vec3){0, 2, 0});

    SRE_Col_load(block1.collider);
    SRE_Col_load(kms_uwu.collider);

    glEnable(GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.03125f, 0.125f, 1.0f);
    

    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "proj"), 1, GL_FALSE, cam.proj);
    glUniformMatrix4fv(SRE_Get_uniform_location(&program, "view"), 1, GL_FALSE, cam.view);


    sre_light light1;
    glm_vec4_copy((vec4){0.0f, 7.0f, 20.0f, 1.0f}, light1.position);
    light1.radius = 20.0f;
    light1.color.rgba = RGBA(255, 255, 255, 0);

    SRE_Light_push_uniform_array(&light1, program);
    SRE_Light_push_uniform_array(&light1, program_alpha_clip);

    sre_light light2;
    glm_vec4_copy((vec4){16.0f, 4.5f, -5.0f, 1.0f}, light2.position);
    light2.radius = 20.0f;
    light2.color.rgba = RGBA(191, 0, 0, 0);

    //SRE_Light_push_uniform_array(&light2, program);
    //SRE_Light_push_uniform_array(&light2, program_alpha_clip); 
 
    sre_light light3;
    glm_vec4_copy((vec4){10.0f, 8.5f, -10.5f, 1.0f}, light3.position);
    light3.radius = 30.0f;
    light3.color.rgba = RGBA(0, 0, 127, 0);

    //SRE_Light_push_uniform_array(&light3, program);
    //SRE_Light_push_uniform_array(&light3, program_alpha_clip);

    sre_light sun;
    glm_vec4_copy((vec4){0.0f, 1.0f, 0.0f, 0.0f}, sun.position);
    sun.color.rgba = RGBA(0x7f, 0x7f, 0x7f, 0xff);
    sun.radius = 0.0f;

    SRE_Light_push_uniform_array(&sun, program);
    SRE_Light_push_uniform_array(&sun, program_alpha_clip);

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);

    printf("vendor: %s\nrenderer %s\n", vendor, renderer);

    int running = 1;
    SDL_Event event;

    Uint64 last = 0;
    float dt = 0; 
    float lx = 0;
    float ly = 0;
    float lz = 0;

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // main loop

    float scale_factor = 2.0f;
    Uint8 cam_updated = 0;
    Sint32 xrel = 0;
    Sint32 yrel = 0;

    sre_terrain terrain;
    generate_terrain_vertices(100, 100, &terrain, 10);

    Uint64 now = SDL_GetPerformanceCounter();
    while (running)
    {
        cam_updated = 0;
        xrel = 0;
        yrel = 0;
        last = now;
        now = SDL_GetPerformanceCounter();
        dt = (float)((now - last) * 1000 / (float)SDL_GetPerformanceFrequency()) * 0.001;
        

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
            {
                int numkeys;
                Uint8 *keystates = SDL_GetKeyboardState(&numkeys);
                kms_uwu.cntl->flags = CTL_DIR_FRONT * keystates[SDL_SCANCODE_UP] |
                                    CTL_DIR_BACK * keystates[SDL_SCANCODE_DOWN] |
                //                    CTL_DIR_UP * keystates[SDL_SCANCODE_SPACE] |
                //                    CTL_DIR_DOWN * keystates[SDL_SCANCODE_LSHIFT] |
                                    CTL_DIR_LEFT * keystates[SDL_SCANCODE_LEFT] |
                                    CTL_DIR_RIGHT * keystates[SDL_SCANCODE_RIGHT];
                cam.cntl->flags = CTL_DIR_FRONT * keystates[SDL_SCANCODE_W] |
                                    CTL_DIR_BACK * keystates[SDL_SCANCODE_S] |
                                    CTL_DIR_UP * keystates[SDL_SCANCODE_SPACE] |
                                    CTL_DIR_DOWN * keystates[SDL_SCANCODE_LSHIFT] |
                                    CTL_DIR_LEFT * keystates[SDL_SCANCODE_A] |
                                    CTL_DIR_RIGHT * keystates[SDL_SCANCODE_D];
                if (keystates[SDL_SCANCODE_ESCAPE])
                {
                    running = 0;
                    break;
                }
                break;
            }
            case SDL_MOUSEMOTION:
            {
                cam.cntl->flags = cam.cntl->flags | CTL_MOUSE_MOVEMENT;
                kms_uwu.cntl->flags = kms_uwu.cntl->flags | CTL_MOUSE_MOVEMENT;
                xrel = event.motion.xrel;
                yrel = event.motion.yrel;
                break;
            }
            case SDL_KEYUP:
            {
                int numkeys;
                Uint8 *keystates = SDL_GetKeyboardState(&numkeys);
                kms_uwu.cntl->flags = CTL_DIR_FRONT * keystates[SDL_SCANCODE_UP] |
                                    CTL_DIR_BACK * keystates[SDL_SCANCODE_DOWN] |
                //                    CTL_DIR_UP * keystates[SDL_SCANCODE_SPACE] |
                //                    CTL_DIR_DOWN * keystates[SDL_SCANCODE_LSHIFT] |
                                    CTL_DIR_LEFT * keystates[SDL_SCANCODE_LEFT] |
                                    CTL_DIR_RIGHT * keystates[SDL_SCANCODE_RIGHT];
                cam.cntl->flags = CTL_DIR_FRONT * keystates[SDL_SCANCODE_W] |
                                    CTL_DIR_BACK * keystates[SDL_SCANCODE_S] |
                                    CTL_DIR_UP * keystates[SDL_SCANCODE_SPACE] |
                                    CTL_DIR_DOWN * keystates[SDL_SCANCODE_LSHIFT] |
                                    CTL_DIR_LEFT * keystates[SDL_SCANCODE_A] |
                                    CTL_DIR_RIGHT * keystates[SDL_SCANCODE_D];
                break;
            }
            default:
                break;
            }
            
        }
        if (cam.cntl->flags)
        {
            SRE_Cam_update_pos(&cam, dt, (float)xrel / (float)0x1fu, (float)yrel / (float)0x1fu);

            SRE_Cam_set_view_mat(&cam);
            SRE_Cam_set_proj_mat(&cam);

            glProgramUniformMatrix4fv(program.id, SRE_Get_uniform_location(&program, "proj"), 1, GL_FALSE, cam.proj);
            glProgramUniformMatrix4fv(program.id, SRE_Get_uniform_location(&program, "view"), 1, GL_FALSE, cam.view);
        
            glProgramUniformMatrix4fv(program_alpha_clip.id, SRE_Get_uniform_location(&program_alpha_clip, "proj"), 1, GL_FALSE, cam.proj);
            glProgramUniformMatrix4fv(program_alpha_clip.id, SRE_Get_uniform_location(&program_alpha_clip, "view"), 1, GL_FALSE, cam.view);       
        }
        if (kms_uwu.cntl->flags)
        {
            prop_control(&kms_uwu, dt, (float)xrel / (float)0x1fu, (float)yrel / (float)0x1fu);
            sre_coldat_aabb *data1 = kms_uwu.collider->data;
            sre_coldat_aabb *data2 = block1.collider->data;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // draw to postprocess framebuffer
        
        glUniform1ui(SRE_Get_uniform_location(&program, "time"), now);

        sre_collider expanded_col;
        sre_coldat_aabb col_data;
        expanded_col.data = &col_data;
        SRE_Expand_collision(kms_uwu.collider, block1.collider, &expanded_col);
        SRE_Draw_collider(&expanded_col, program);

        draw_prop(kms_uwu, program);
        draw_prop(block1, program);
        draw_terrain(terrain, program);
        glUseProgram(program_alpha_clip.id);

        draw_to_main_framebuffer(postproc_framebuffer, postproc_program, width, height);
        framebuffer_bind(postproc_framebuffer, program.id);
        SDL_GL_SwapWindow(window);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("deleting shader programs...   ");
    SRE_Delete_shader_program(&program);
    SRE_Delete_shader_program(&program_alpha_clip);
    printf("[done]\n");
    printf("deleting postproc framebuffer...   ");
    delete_framebuffer(&postproc_framebuffer);
    printf("[done]\n");
    printf("deleting postproc render plane...   ");
    delete_postproc_plane();
    printf("[done]\n");
    printf("deleting geometry...   ");
    delete_terrain_vertices(&terrain);
    delete_model_obj(&brick_block_model);
    SRE_Delete_gl_mesh(&brick_mesh);
    printf("[done]\n");

    return 0;
}
