#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex_coords;

out vec2 tex_coords_out;

void main()
{
    gl_Position = vec4(pos.xyz, 1.0); 
    tex_coords_out = tex_coords;
}  