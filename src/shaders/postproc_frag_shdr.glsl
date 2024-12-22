#version 460 core
// input
in vec2 uv;
//output
out vec3 color;
// unifroms
uniform sampler2D rendered_texture;
uniform uint screen_width;
uniform uint screen_height;
uniform uint time = 0;
// dither matrix
mat4 dither_mat = mat4(
    vec4(0, 0.5, 0.125, 0.625),
    vec4(0.75, 0.25, 0.875, 0.375),
    vec4(0.1875, 0.6875, 0.0625, 0.5625),
    vec4(0.9375, 0.4375, 0.8125, 0.3125));
// main
void main(){
    vec3 render_result = texture(rendered_texture, uv).xyz;
    uint i = (uint(ceil(480 * uv.x)) + (time >> 25)) & 0x3;
    uint j = uint(ceil(360 * uv.y)) & 0x3;
    vec3 th = vec3(dither_mat[i][j] - 0.5);
    float r = 0.03125;
    th *= r;
    color = round((render_result + th)  *  16) * 0.0625;
}