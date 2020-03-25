#version 430 core

out vec4 fragColor;

in vec2 texCoords;
uniform sampler2D color_texture;

void main()
{
   fragColor = vec4(vec3(texture(color_texture, texCoords).y), 1.0);
}