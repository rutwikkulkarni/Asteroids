#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex_coords;

uniform mat4 ortho;
out vec2 tex_coords_out;

void main(){
	tex_coords_out = tex_coords;
    gl_Position = ortho * vec4(pos.xyz, 1.0);
}