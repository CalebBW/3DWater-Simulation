#version 430 core

//Use these to adjust the surface visuals
//----------------------------------------------------
vec4 waterColor = vec4(0.875, 0.875, 1.0, 1.0);
vec4 fogColor = vec4(0.333, 0.247, 0.137, 1.0); //Mud
vec4 turbulenceColor = vec4(1.0, 0.89, 0.89, 1.0f);
//----------------------------------------------------

in vec3 fragPos;
in vec2 texCoords;
in vec4 vertColor; //Only 0 or 1
in vec4 glPos;

out vec4 fragColor;

uniform vec3 cameraPos;
uniform sampler2D height_texture;
uniform sampler2D surfaceData_texture;
uniform sampler2D scene_texture;
uniform sampler2D depth_texture;
uniform samplerCube cubemap_texture;

uniform float fogDensity;
uniform float turbulenceStrength;
uniform float refractionStrength;
uniform float reflectionStrength;

//uniform float fogDensity = 1.5f;
//uniform float turbulenceStrength = 0.0f;
//uniform float refractionStrength = 0.25f;
//uniform float reflectionStrength = 0.35f;

//Unused for now
vec3 lightPos = vec3(5.0, -3.0, 10.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);
vec3 ambientColor = vec3(0.1, 0.075, 0.075);

void main()
{
   //Normal offset
   vec4 surfaceData = texture(surfaceData_texture, texCoords);
   vec2 surfaceNormal = surfaceData.xy;
   vec3 normal = normalize(vec3(0.0, 1.0, 0.0) + vec3(surfaceNormal.x, 0.0, surfaceNormal.y));

   //Surface rate of change
   float surfaceVel = pow(surfaceData.z * 100.0, 2);

   //Linear depth from depth texture
   float near = 0.1;
   float far = 100.0;
   vec2 screenCoords = 0.5 * glPos.xy / glPos.w + 0.5;
   float ndc = texture(depth_texture, screenCoords).r * 2.0 - 1.0; 
   float imageDepth = (2.0 * near * far) / (far + near - ndc * (far - near));

   //Linear depth of fragment
   ndc = (glPos.z / glPos.w) * 2.0 - 1.0;
   float surfaceDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
   float depthDiff = imageDepth - surfaceDepth;
   depthDiff *= 0.35; //This constant has a big effect depending on how far away our water is.
   float depthf = pow(depthDiff*0.1, 2);

   //Water surface + refraction
   surfaceNormal *= refractionStrength;
   vec4 surfaceColor = texture(scene_texture, screenCoords + surfaceNormal) * waterColor;

   //Turbulence
   surfaceColor += turbulenceColor * surfaceVel * turbulenceStrength;

   //Reflection
   vec3 I = normalize(fragPos - cameraPos);
   vec3 R = reflect(I, normal);
   vec4 reflectionColor = vec4(texture(cubemap_texture, R).rgb, 1.0);

   //Adjust the color based on water depth
   surfaceColor = mix(surfaceColor, fogColor, fogDensity * depthf);

   //Mix with reflections last so that the reflection color isn't effected by other factors
   vec4 finalColor = mix(surfaceColor, reflectionColor, reflectionStrength * depthf) * vertColor;

   fragColor = clamp(finalColor, vec4(0.0), vec4(1.0));
}