#version 460 core
// input
layout(location=0) in vec3 pos3;
layout(location=1) in vec2 uv2;
layout(location=2) in vec3 nrms3;
layout(location=3) in uvec4 bones4;
layout(location=4) in vec4 w4;
// output
out smooth vec3 lit;
out smooth vec3 spec;
out vec2 v_uv2;
// uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform struct Light
{
    vec3 position;
    uint color;
    float radius;
} lights[16], inf_lights[8];
uniform uint light_count = 0;
uniform uint inf_light_count = 0;
uniform uint ka = 0xff3f3f3f;
uniform uint kd = 0xff3f3f3f;
uniform uint ks = 0xff3f3f3f;
uniform float ns = 900.000000;
uniform uint keyframe_count = 0;
uniform float t_normalized;
uniform mat4 bone_matrices_kf1[100];
uniform mat4 bone_matrices_kf2[100];
uniform uint time = 0;
uniform bool animated = false;
// main
void main(){
    vec4 pos4 = vec4(pos3 , 1.0f);
    vec4 nrms4 = normalize(vec4(nrms3, 1.0f));
    vec4 pos_kf1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4 pos_kf2 = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    vec4 nrms_kf1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4 nrms_kf2 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    vec4 pos_lerp;
    vec4 nrms_lerp;

    if(animated) {
        for(uint i = 0; i < 4; i++) 
        {
            float w = w4[i];
            if(w < 0.001) 
            {
                continue;
            }
            uint bone = bones4[i];

            mat4 rot_only_kf1 = bone_matrices_kf1[bone];
            mat4 rot_only_kf2 = bone_matrices_kf2[bone];
            rot_only_kf1[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
            rot_only_kf2[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

            pos_kf1 += (bone_matrices_kf1[bone] * pos4) * w;
            pos_kf2 += (bone_matrices_kf2[bone] * pos4) * w;

            nrms_kf1 += (rot_only_kf1 * nrms4) * w;
            nrms_kf2 += (rot_only_kf2 * nrms4) * w;
        }
        pos_lerp = mix(pos_kf1, pos_kf2, t_normalized);
        nrms_lerp = mix(nrms_kf1, nrms_kf2, t_normalized);
        nrms_lerp.w = 1.0f;
        pos_lerp.w = 1.0f;
    }
    else
    {
        pos_lerp = pos4;
        nrms_lerp = nrms4;
    }

    vec4 model_pos = model * pos_lerp;
    
    mat4 rot_only = model;
    rot_only[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    vec4 model_nrms = rot_only * nrms_lerp;
    vec4 view_pos = view * model_pos;

    //model_pos.xyz = floor(model_pos.xyz * 8.0f) * 0.125;

    view_pos.xyz = floor(view_pos.xyz * 8.0f) * 0.125;

    lit = vec3(0.0f);
    spec = vec3(0.0f);
    vec3 vdir = normalize(-view_pos.xyz);
    vec4 ks4 = unpackUnorm4x8(ks);

    for(uint i = 0; i < inf_light_count; i++)
    {
        Light l = inf_lights[i];

        vec4 colvec4 = unpackUnorm4x8(l.color);
        vec3 dir = l.position;
        float lproj = max(dot(model_nrms.xyz, dir), 0);
        
        lit += clamp(colvec4.xyz * lproj, 0.0f, 1.0f);
        vec3 refl = 2 * lproj * model_nrms.xyz - dir;

        vec3 tmp_spec = colvec4.xyz * ks4.x * pow(max(dot(normalize(refl), vdir), 0), ns);
        spec += clamp(tmp_spec, 0.0f, 1.0f);
    }

    for(uint i = 0; i < light_count; i++)
    {
        Light l = lights[i];
        vec3 ray = l.position - model_pos.xyz;
        float d = length(ray);
        if(d * 0.25 >= l.radius) continue;

        vec4 colvec4 = unpackUnorm4x8(l.color);
        vec3 dir = normalize(ray);
        float lproj = max(dot(model_nrms.xyz, dir), 0);
        float att = clamp((l.radius * 4) / (d * d), 0.0f, 1.0f);
        
        lit += clamp(colvec4.xyz * lproj * att, 0.0f, 1.0f);
        vec3 refl = 2 * lproj * model_nrms.xyz - dir;
        
        if(d * d * 0.25 >= l.radius) continue;
        vec3 tmp_spec = colvec4.xyz * ks4.x * pow(max(dot(normalize(refl), vdir), 0), ns);
        spec += clamp(tmp_spec, 0.0f, 1.0f);
    }

    lit = min(lit, vec3(1.0f));
    spec = min(spec, vec3(1.0f));
    v_uv2 = uv2;
    gl_Position = proj * view_pos;
}