#version 460 core
// input
in smooth vec3 lit;
in smooth vec3 spec;
in noperspective vec2 v_uv2;
in noperspective vec3 vpos;
// uniforms
uniform uint ka = 0xff3f3f3f;
uniform uint kd = 0xff7f7f7f;
uniform uint use_diffuse_tex = 0;
uniform sampler2D diffuse_tex_sampler;
uniform uint time = 0;
// output
layout(location = 0) out vec4 col;
// main
void main(){
    vec4 diffuse_tex_data;
    if(use_diffuse_tex > 0){
        //vec2 uv = vec2(v_uv2.x + float((time >> 20) & 0xff) / 256.0, v_uv2.y);
        diffuse_tex_data = texture(diffuse_tex_sampler, v_uv2);
    }
    else {
        diffuse_tex_data = vec4(1.0f);
    }
    vec4 ka4 = unpackUnorm4x8(ka);
    vec4 kd4 = unpackUnorm4x8(kd);
    vec3 ambient_light = kd4.xyz * 0.2f;
    col = vec4(min(diffuse_tex_data.xyz * kd4.xyz * (lit + ambient_light * ka4.xyz) + spec , vec3(1.0f)), diffuse_tex_data.w);
}