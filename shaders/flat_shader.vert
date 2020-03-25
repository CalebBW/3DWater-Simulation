#version 430 core

uniform mat4 ModelViewProjection_mat;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;

void main()
{
   //Transform the vertex position
   gl_Position = ModelViewProjection_mat * vec4(vPosition, 1.0f);
}