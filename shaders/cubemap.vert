#version 430 core

layout (location = 0) in vec3 vPosition;

out vec3 texCoords;

uniform mat4 projection_mat;
uniform mat4 view_mat;

void main()
{
    texCoords = vPosition;
    gl_Position = projection_mat * view_mat * vec4(vPosition, 1.0);
} 