#version 410 core

out vec4 final_colour;

in vec2 tex_coords_out;
uniform sampler2D sprite;

void main(){
	final_colour = texture(sprite, tex_coords_out);
	if(final_colour.r == 0 && final_colour.g == 0 && final_colour.b == 0) discard;
}