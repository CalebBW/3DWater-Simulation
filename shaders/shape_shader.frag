#version 430 core

in vec3 fragPos;
in vec3 normal;
in vec2 texCoords;

out vec4 fragColor;

uniform vec3 cameraPos;
uniform sampler2D color_texture;

vec3 lightPos = vec3(15.0, 45.0, -45.0);
vec3 lightColor = vec3(0.945f, 0.855f, 0.843f);
vec3 ambientColor = vec3(0.4f);

void main()
{
   vec3 lightVec = normalize(lightPos - fragPos);
   float diff = max(dot(normal, lightVec), 0.0);

   /*
   //Basic specular
   float specStrength = 0.25;
   vec3 viewVec = normalize(cameraPos - fragPos);
   vec3 reflectVec = reflect(-lightVec, normal);
   float spec = pow(max(dot(viewVec, reflectVec), 0.0), 64);
   vec3 specularColor = specStrength * spec * lightColor;
   */

   vec3 diffuse = diff * lightColor;

   vec3 texColor = texture(color_texture, texCoords).rgb;   
   //vec4 color = vec4(texColor.rgb * (diffuse + ambientColor + specularColor), 1.0f);
   vec4 color = vec4(texColor.rgb * (diffuse + ambientColor), 1.0f);

   fragColor = color;
}