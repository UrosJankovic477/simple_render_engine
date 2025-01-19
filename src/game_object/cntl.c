#include <sre/game_object/cntl.h>
#include <sre/event/event.h>
#define USE_MATH_DEFINES

sre_control_listener main_control_listener;

#define SRE_CONTROL_FLAG_FORWARD             (1)
#define SRE_CONTROL_FLAG_BACK                (1 << 1)
#define SRE_CONTROL_FLAG_RIGHT               (1 << 2)
#define SRE_CONTROL_FLAG_LEFT                (1 << 3)
#define SRE_CONTROL_FLAG_UP                  (1 << 4)
#define SRE_CONTROL_FLAG_DOWN                (1 << 5)
#define SRE_CONTROL_FLAG_INTERACT            (1 << 6)
#define SRE_CONTROL_FLAG_USE                 (1 << 7)
#define SRE_CONTROL_FLAG_PAUSE               (1 << 8)
#define SRE_CONTROL_FLAG_MOUSE_MOTION        (1 << 9)
// settings and debug options

#define SRE_CONTROL_FLAG_NO_CLIP             (1 << 29)
#define SRE_CONTROL_FLAG_RELATIVE_MOVEMENT   (1 << 30)
#define SRE_CONTROL_FLAG_MOUSE_INVERTED      (1 << 31)


#define SRE_CONTROL_SETTINGS_FLAGS (\
SRE_CONTROL_FLAG_MOUSE_INVERTED | \
SRE_CONTROL_FLAG_RELATIVE_MOVEMENT\
)

static const uint32_t flag_array[32] = {
    SRE_CONTROL_FLAG_FORWARD        ,
    SRE_CONTROL_FLAG_BACK           ,
    SRE_CONTROL_FLAG_LEFT           ,
    SRE_CONTROL_FLAG_RIGHT          ,
    SRE_CONTROL_FLAG_UP             ,
    SRE_CONTROL_FLAG_DOWN           ,
    SRE_CONTROL_FLAG_INTERACT       ,
    SRE_CONTROL_FLAG_USE            ,
    SRE_CONTROL_FLAG_PAUSE          ,
};

void SRE_Control_create(sre_control_listener *listener, float speed, float sensitivity, sre_control_handler control_handler)
{
    listener->velocity = speed;
    listener->sensitivity = sensitivity;
    listener->control_flags = 0;
    glmc_vec2_zero(listener->mouse_motion);
    listener->control_handler = control_handler;
    SRE_Orientation_create(&listener->orientation);

}


void SRE_Keyboard_handler_translate(sre_control_listener *listener, float dt)
{
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    if (keyboard_state[SDL_SCANCODE_ESCAPE])
    {
        SRE_App_quit();
        return;
    }
    if (keyboard_state[SDL_SCANCODE_F12])
    {
        SRE_Scene_export("../resources/scenes/scene.scn");
    }
    if (keyboard_state[SDL_SCANCODE_F11])
    {
        SRE_Scene_import("../resources/scenes/scene.scn");
    }
    
    
    listener->distance = dt * listener->velocity;
    listener->control_flags &= SRE_CONTROL_SETTINGS_FLAGS | SRE_CONTROL_FLAG_MOUSE_MOTION;
    for (size_t i = 0; i < SRE_CONTROL_ACTION_COUNT; i++)
    {
        Uint32 pressed = (Uint32)keyboard_state[listener->keyboard_config[i]];
        Uint32 flag = flag_array[i];
        listener->control_flags |= flag * pressed;
    }
}

void SRE_Mouse_movement_handler_rotate_xy(SDL_Event event, sre_control_listener *listener, float dt)
{
    listener->control_flags |= SRE_CONTROL_FLAG_MOUSE_MOTION;
    float sdt = listener->sensitivity * dt;
    sre_euler euler;
    listener->mouse_motion[0] += sdt * event.motion.xrel;
    listener->mouse_motion[1] += sdt * event.motion.yrel;


}

int SRE_Control_handler_default(SDL_Event event, float dt)
{
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
        SRE_Keyboard_handler_translate(&main_control_listener, dt);
        return SRE_SUCCESS;
    }
    else if (event.type == SDL_MOUSEMOTION)
    {
        SRE_Mouse_movement_handler_rotate_xy(event, &main_control_listener, dt);
        return SRE_SUCCESS;
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
    {
        return SRE_SUCCESS;
    }


    return SRE_UNRECOGNIZED_EVENT;
}

void SRE_Control_update(sre_control_listener *listener)
{
    if ((listener->control_flags & (~SRE_CONTROL_SETTINGS_FLAGS)) == 0)
    {
        return;
    }

    sre_event event;
    sre_transform_event event_arg;
    event.args = (sre_event_args*)&event_arg;
    event.code = SRE_TRANSFORM_EVENT;
    event.sender = (sre_game_object*)listener;
    event.args->transform.trans_flags = 0;

    SRE_Trans_copy(listener->moving_object->transform, &event.args->transform.start_trans);
    SRE_Trans_copy(listener->moving_object->transform, &event.args->transform.end_trans);

    if (listener->control_flags & SRE_CONTROL_FLAG_MOUSE_MOTION)
    {
        event.args->transform.trans_flags | SRE_TRANS_ROTATE;
        listener->control_flags ^= SRE_CONTROL_FLAG_MOUSE_MOTION;
        sre_euler rotation;
        rotation.yaw = listener->mouse_motion[0] + listener->moving_object->transform.rotation.yaw;
        rotation.yaw = fmodf(rotation.yaw, GLM_PI * 2);
        if ((listener->control_flags & SRE_CONTROL_FLAG_MOUSE_INVERTED))
        {
            rotation.pitch = -listener->mouse_motion[1];
        }
        else
        {
            rotation.pitch = listener->mouse_motion[1];
        }
        rotation.pitch += listener->moving_object->transform.rotation.pitch;
        rotation.pitch = glm_clamp(rotation.pitch, -GLM_PI_2f + 0.1f, GLM_PI_2f - 0.1f);
        rotation.roll = 0.0f;
        glmc_vec3_copy(rotation.vector, event.args->transform.end_trans.rotation.vector);
        mat4 rot_y;
        glmc_rotate_y(GLM_MAT4_IDENTITY, -rotation.yaw, rot_y);
        glmc_mat4_mulv3(rot_y, (vec3) {0.0f, 0.0f, 1.0f}, 1.0f, listener->orientation.dir);
        glmc_mat4_mulv3(rot_y, (vec3) {1.0f, 0.0f, 0.0f}, 1.0f, listener->orientation.right);
        //SRE_Orientation_update(&listener->orientation, event.args->transform.end_trans.rotation);
        glmc_vec2_zero(listener->mouse_motion);
    }

    vec3 translation;
    glmc_vec3_zero(translation);
    if ((listener->control_flags & SRE_CONTROL_FLAG_FORWARD) &&
    !(listener->control_flags & SRE_CONTROL_FLAG_BACK))
    {
        event.args->transform.trans_flags |= SRE_TRANS_TRANSLATE;
        glmc_vec3_sub(translation, listener->orientation.dir, translation);
    }
    else if ((listener->control_flags & SRE_CONTROL_FLAG_BACK) &&
    !(listener->control_flags & SRE_CONTROL_FLAG_FORWARD))
    {
        event.args->transform.trans_flags |= SRE_TRANS_TRANSLATE;
        glmc_vec3_add(translation, listener->orientation.dir, translation);
    }
    if ((listener->control_flags & SRE_CONTROL_FLAG_RIGHT) &&
    !(listener->control_flags & SRE_CONTROL_FLAG_LEFT))
    {
        event.args->transform.trans_flags |= SRE_TRANS_TRANSLATE;
        glmc_vec3_add(translation, listener->orientation.right, translation);
    }
    else if ((listener->control_flags & SRE_CONTROL_FLAG_LEFT) &&
    !(listener->control_flags & SRE_CONTROL_FLAG_RIGHT))
    {
        event.args->transform.trans_flags |= SRE_TRANS_TRANSLATE;
        glmc_vec3_sub(translation, listener->orientation.right, translation);
    }
    if ((listener->control_flags & SRE_CONTROL_FLAG_UP) &&
    !(listener->control_flags & SRE_CONTROL_FLAG_DOWN))
    {
        event.args->transform.trans_flags |= SRE_TRANS_TRANSLATE;
        glmc_vec3_add(translation, listener->orientation.up, translation);
    }
    else if ((listener->control_flags & SRE_CONTROL_FLAG_DOWN) &&
    !(listener->control_flags & SRE_CONTROL_FLAG_UP))
    {
        event.args->transform.trans_flags |= SRE_TRANS_TRANSLATE;
        glmc_vec3_sub(translation, listener->orientation.up, translation);
    }
    glmc_vec3_scale(translation, listener->distance, translation);
    glmc_vec3_add(translation, listener->moving_object->transform.translation, event.args->transform.end_trans.translation);

    if ((listener->control_flags & (uint32_t)SRE_CONTROL_FLAG_NO_CLIP) == 0)
    {
        event.args->transform.trans_flags |= SRE_TRANS_CHECK_COLLISION;
    }

    SRE_Group_handle_user_event(listener->moving_object, event);
}

void SRE_Controls_handler(SDL_Event event, float dt)
{
    main_control_listener.control_handler(event, dt);
}
