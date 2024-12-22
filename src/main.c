#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <cglm/types.h>
#include <cglm/call/cam.h>
#include <cglm/call/mat4.h>
#include <sre/sre.h>

#define _USE_MATH_DEFINES
#include <math.h>


#define DEG2RAD(x) M_PI / 180 * (x)


int main(int argc, char **argv)
{
    // Init

    int status = SRE_Bump_create(&main_allocator, (size_t)0xffff);
    if (status != SRE_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    int width = 480 * 2;
    int height = 360 * 2;
    SDL_Window *window =
    SDL_CreateWindow(   "TEST-APPLICATION",
                        0,
                        0,
                        width, height,
                        SDL_WINDOW_OPENGL);
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    SDL_GL_MakeCurrent(window, glcontext);

    glEnable (GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    SDL_SetWindowMouseGrab(window, SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    stbi_set_flip_vertically_on_load(1);

    sre_framebuffer postproc_framebuffer;
    SRE_Framebuffer_init(&postproc_framebuffer, 240, 180, GL_RGB);

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

    sre_program program;

    SRE_Create_shader_program(&program, vertex_shader, fragment_shader);

    SRE_Init_postproc();

    glUseProgram(program.id);

    sre_camera *main_camera;
    sre_control_listener main_control_listener;

    SRE_Camera_create(&main_camera, "main_camera", (vec3) {0, 0, 8}, (vec3) {0, 0, 0}, 0.01f, 200.0f, ((float)width / (float)height), glm_rad(70.0f));
    SRE_Control_create(&main_control_listener, 10, 0.5, SRE_Control_handler_default);
    SRE_Control_listener_set_main(&main_control_listener);

    // temporarely hardcode keyboard configuration

    main_control_listener.keyboard_config[SRE_CONTROL_ACTION_FORWARD] = SDL_SCANCODE_W;
    main_control_listener.keyboard_config[SRE_CONTROL_ACTION_LEFT] = SDL_SCANCODE_A;
    main_control_listener.keyboard_config[SRE_CONTROL_ACTION_BACK] = SDL_SCANCODE_S;
    main_control_listener.keyboard_config[SRE_CONTROL_ACTION_RIGHT] = SDL_SCANCODE_D;
    main_control_listener.keyboard_config[SRE_CONTROL_ACTION_INTERACT] = SDL_SCANCODE_F;
    main_control_listener.keyboard_config[SRE_CONTORL_ACTION_UP] = SDL_SCANCODE_SPACE;
    main_control_listener.keyboard_config[SRE_CONTROL_ACTION_DOWN] = SDL_SCANCODE_LSHIFT;

    SRE_Importer_init_default();
    SRE_Import_asset("../resources/models/python_testing.sarm");
    SRE_Import_asset("../resources/models/python_testing.smtl");
    SRE_Import_asset("../resources/models/python_testing.smsh");

    SRE_Import_asset("../resources/models/checkerboard.smtl");
    SRE_Import_asset("../resources/models/checkerboard.smsh");
    SRE_Import_collision("../resources/models/checkerboard.scol");

    //SRE_Camera_set_view_mat(main_camera);
    SRE_Camera_set_proj_mat(main_camera);

    glEnable(GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.03125f, 0.125f, 1.0f);

    //glUniformMatrix4fv(SRE_Get_uniform_location(&program, "proj"), 1, GL_FALSE, main_camera->proj);
    //glUniformMatrix4fv(SRE_Get_uniform_location(&program, "view"), 1, GL_FALSE, main_camera->view);

    sre_light *light1;
    sre_rgba color;
    color.r = 0;
    color.g = 255;
    color.b = 255;
    color.a = 0;
    SRE_Light_create(&light1, "light1", (vec3){0.0f, 0.0f, 0.0f}, color, 5.0f, false);
    SRE_Light_push_uniform_array(light1, program);

    sre_light *light2;
    SRE_Light_create(&light2, "light2", (vec3){1.0f, 0.2f, 0.2f}, (sre_rgba)SRE_WHITE, 0.0f, true);
    SRE_Light_push_uniform_array(light2, program);

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);

    printf("vendor: %s\nrenderer %s\n", vendor, renderer);

    SDL_Event event;

    Uint64 last = 0;
    float dt = 0;
    float lx = 0;
    float ly = 0;
    float lz = 0;

    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // main loop

    float scale_factor = 2.0f;
    Uint8 cam_updated = 0;
    Sint32 xrel = 0;
    Sint32 yrel = 0;

    //sre_terrain terrain;
    //generate_terrain_vertices(100, 100, &terrain, 10);

    sre_game_object *squishy_mesh, *room_mesh, *collision;
    SRE_Game_object_get("Cube", &squishy_mesh);
    SRE_Game_object_get("checkerboard_room", &room_mesh);
    SRE_Game_object_get("collision", &collision);

    sre_group *squishy, *room, *camera_group;
    SRE_Group_create(&squishy, "squishy_group");
    SRE_Group_create(&room, "room_group");
    //SRE_Group_create(&camera_group, "camera_group");
    main_camera->lookat = &squishy->transform.translation;

    SRE_Group_add_component(squishy, (sre_game_object*)main_camera);
    main_control_listener.moving_object = squishy;

    SRE_Group_add_component(room, room_mesh);
    SRE_Group_add_component(room, collision);


    sre_collider *squishy_collider;
    SRE_Aabb_from_mesh(squishy_mesh->mesh, &squishy_collider);
    SRE_Group_add_component(squishy, squishy_mesh);
    squishy->collider = squishy_collider;
    sre_action wiggle;
    SRE_Action_get_by_name(squishy_mesh->mesh.armature, "wiggle", &wiggle);
    SRE_Action_set_active(&wiggle);

    SRE_Group_load(squishy);
    SRE_Group_load(room);

    Uint64 now = SDL_GetPerformanceCounter();
    SRE_App_start();
    while (SRE_App_running())
    {
        cam_updated = 0;
        xrel = 0;
        yrel = 0;
        last = now;
        now = SDL_GetPerformanceCounter();
        dt = (double)((now - last) / (double)SDL_GetPerformanceFrequency());

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
            {
                SRE_App_quit();
                break;
            }
            case SDL_WINDOWEVENT_FOCUS_GAINED:
            {
                SDL_SetWindowMouseGrab(window, SDL_TRUE);
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_TRUE);
                break;
            }
            case SDL_KEYDOWN:
            {
                SRE_Controls_handler(event, dt);
                break;
            }
            case SDL_MOUSEMOTION:
            {
                SRE_Controls_handler(event, dt);
                break;
            }
            case SDL_KEYUP:
            {
                SRE_Controls_handler(event, dt);
                break;
            }
            default:
                break;
            }

        }

        SRE_Control_update(&main_control_listener);

        glUseProgram(program.id);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // draw to postprocess framebuffer
        glUniformMatrix4fv(SRE_Get_uniform_location(&program, "proj"), 1, GL_FALSE, main_camera->proj);
        glUniformMatrix4fv(SRE_Get_uniform_location(&program, "view"), 1, GL_FALSE, main_camera->view);
        glUniform1ui(SRE_Get_uniform_location(&program, "time"), now);
        SRE_Set_current_keyframes(program, now);
        // Draw here
        SRE_Group_draw(room, program, SRE_GO_MESH);
        SRE_Group_draw(squishy, program, SRE_GO_MESH);

        SRE_Draw_to_main_framebuffer(postproc_framebuffer, postproc_program, width, height);
        SRE_Framebuffer_bind(postproc_framebuffer, program.id);
        SDL_GL_SwapWindow(window);
    }
    SRE_Group_unload(room);
    SRE_Group_unload(squishy);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("deleting shader programs...   ");
    SRE_Delete_shader_program(&program);
    //SRE_Delete_shader_program(&program_alpha_clip);
    printf("[done]\n");
    printf("deleting postproc framebuffer...   ");
    SRE_Delete_framebuffer(&postproc_framebuffer);
    printf("[done]\n");
    printf("deleting postproc render plane...   ");
    SRE_Delete_postproc_plane();
    printf("[done]\n");
    printf("deleting geometry...   ");
    SRE_Bump_destroy(&main_allocator);

    printf("[done]\n");

    return 0;
}
