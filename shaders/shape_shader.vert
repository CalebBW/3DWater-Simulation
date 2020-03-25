#version 430 core

uniform mat4 ModelViewProjection_mat;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoords;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoords;

void main()
{
   //Transform the vertex position
   gl_Position = ModelViewProjection_mat * vec4(vPosition, 1.0f);

   fragPos = vec3(mat4(1.0) * vec4(vPosition, 1.0));
   texCoords = vTexCoords;
   normal = normalize(vNormal);
}