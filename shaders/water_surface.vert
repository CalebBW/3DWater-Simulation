#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;

out vec2 texCoords;
out vec4 vertColor;
out vec3 fragPos;
out vec4 glPos;

uniform mat4 ModelViewProjection_mat;
uniform sampler2D height_texture;

//Only used for color blending.
float maxHeight = 10.0f;

void main()
{
   vec3 newPosition = vPosition;
   float f = texture(height_texture, vTexCoords).y;
   newPosition.y = f;

   //Transform the vertex position
   gl_Position = ModelViewProjection_mat * vec4(newPosition, 1.0f);
   glPos = gl_Position;
   fragPos = vec3(mat4(1.0) * vec4(vPosition, 1.0));

   texCoords = vTexCoords;
 
   //Hide the water at 0 height
   vertColor = mix(vec4(0.0f), vec4(1.0f), step(0.005f, f));
}