#version 460 core
// input
in smooth vec3 lit;
in smooth vec3 spec;
in noperspective vec2 v_uv2;
in noperspective vec3 vpos;
// uniforms
uniform uint ka = 0xffffffff;
uniform uint kd = 0xffffffff;
uniform sampler2D diffuse_tex_sampler;
uniform float alpha_threshold = 0.1f;
uniform uint time = 0;
// output
layout(location = 0) out vec4 col;
// main
void main(){
    vec4 diffuse_tex_data;
    vec2 uv = vec2(v_uv2.x, v_uv2.y);
    diffuse_tex_data = texture(diffuse_tex_sampler, uv);
    if(diffuse_tex_data.w < alpha_threshold) {
        discard;
    }
    vec4 ka4 = unpackUnorm4x8(ka);
    vec4 kd4 = unpackUnorm4x8(kd);
    vec3 ambient_light = kd4.xyz * 0.2f;
    col = vec4(min(diffuse_tex_data.xyz * kd4.xyz * (lit + ambient_light * ka4.xyz) + spec , vec3(1.0f)), diffuse_tex_data.w);  
}