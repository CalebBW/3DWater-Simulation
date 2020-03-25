#version 430 core

out vec4 fragColor;

in vec2 texCoords;
uniform sampler2D imageA_texture;
uniform sampler2D imageB_texture;

void main()
{
   //Since our mask texture may need updating, do not use the z value from the height texture (imageB_texture).
   vec3 color = texture(imageA_texture, texCoords).rgb + vec3(texture(imageB_texture, texCoords).rg, 0.0);

   fragColor = vec4(color, 1.0f);
}