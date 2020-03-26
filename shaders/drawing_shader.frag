#version 430 core

out vec4 fragColor;

in vec2 texCoords;
uniform sampler2D mask_texture;
uniform sampler2D height_texture;

uniform int mouseBtnDown;
uniform vec2 mousePosition;
uniform float delta;

uniform float brushSize = 0.15f;
uniform float brushPower = 25.0f;

void main()
{
   float y = 0.0; //Height color
   float dist = distance(mousePosition, texCoords);
   vec2 prevData = texture(height_texture, texCoords).rg;

   if (mouseBtnDown == 1)
   {
      //Hard brush
      y = step(dist, brushSize) * brushPower * delta;

      //Smooth brush
      //y = smoothstep(brushSize, 0.05*brushSize, dist) * brushPower * delta;
   }

   //Store the mask color in the blue channel
   float maskColor = texture(mask_texture, texCoords).r;
   fragColor = vec4(prevData.x, prevData.y + y, maskColor, 1.0f);
}
