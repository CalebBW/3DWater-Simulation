#version 430 core

precision highp float;

in vec2 texCoords;

layout(location = 0) out vec4 heightColor;
layout(location = 1) out vec4 surfaceData;

uniform sampler2D height_texture;

//uniform float delta;

//Step size for texture sampling (1.0 / dimensions)
float stepsize = 1.0 / 128.0;

//Global acceleration
float g = 0.1;

//Waveform decay constant
float decay = 0.998;

//The variables used are based on this naming convention.
//        [ C ]
//   [ A ][ M ][ B ]
//        [ D ]
//

void main()
{
   //Do nothing if we're in a masked area
   float Hm = texture(height_texture, texCoords).z;
   if (Hm > 0.0)
   {
      heightColor = vec4(0.0, 0.0, Hm, 1.0);
      return;
   }

   //Gather all of the texture samples we need (10 in total)
   //"Velocity" is stored in the red (x) channel, height is stored in the green (y) channel,
   //and mask value is stored in the blue (z) channel.
   vec2 u = vec2(stepsize, 0.0);
   vec3 m = texture(height_texture, texCoords).xyz;
   vec3 a = texture(height_texture, texCoords - u).xyz;
   vec3 b = texture(height_texture, texCoords + u).xyz;
   u = vec2(0.0, stepsize);
   vec3 c = texture(height_texture, texCoords + u).xyz;
   vec3 d = texture(height_texture, texCoords - u).xyz;

   //Any cells sampled in the mask zone should be seen as equal to m (like GL_CLAMP_TO_EDGE)
   //In case of filtering, use the step function so we get 0 or 1 when mixing

   float Ha = step(0.01, a.z);
   float Hb = step(0.01, b.z);
   float Hc = step(0.01, c.z);
   float Hd = step(0.01, d.z);
   a.xy = mix(a.xy, m.xy, Ha);
   b.xy = mix(b.xy, m.xy, Hb);
   c.xy = mix(c.xy, m.xy, Hc);
   d.xy = mix(d.xy, m.xy, Hd);

   //Smooth things out with waveform decay
   m.x *= decay;

   //Calculate the force applied on this cell by the surrounding 4 cells. This force is used as velocity
   //since we don't want to divide by our cell's mass (height). Bad things would happen if we did.
   //It's a bit of a cheat, but still provides a nice proportional acceleration to the surrounding cells.
   float Fm = m.y * g;
   float Favg = (a.y + b.y + c.y + d.y) * g * 0.25; // (Fa + Fb + Fc + Fd) / 4
   float deltaVm = (Favg - Fm); // / m.y;

   //Add in our new velocity and height
   m.x += deltaVm;
   m.y += m.x;

   //Clamp height to positive values
   m.y = max(0.0, m.y);

   //If the cell's height was reduced to 0, then zero out its velocity
   m.x *= sign(m.y);

   //Water column height
   heightColor = vec4(m.x, m.y, m.z, 1.0);

   //---------------------------------------------------
   //Surface Data
   //---------------------------------------------------
   vec2 normal = vec2( (a.y - b.y), (d.y - c.y) );
   //surfaceData = vec4(normal, abs(deltaVm), 1.0f);
   surfaceData = vec4(normal, abs(m.x), 1.0f);
}
