#version 410 core

out vec4 frag_colour;
in vec2 tex_coords_out;

uniform sampler2D final_texture;

void main(){
	frag_colour = vec4(texture(final_texture, tex_coords_out).rgb, 1.0f);
}