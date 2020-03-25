#version 430 core

out vec4 fragColor;

in vec2 texCoords;
uniform sampler2D mask_texture;

uniform int mouseBtnDown;
uniform vec2 mousePosition;
uniform float delta;

uniform float brushSize = 0.15f;
uniform float brushPower = 25.0f;

void main()
{
   float y = 0.0; //Height color
   float dist = distance(mousePosition, texCoords);
   float xDist = distance(mousePosition.x, texCoords.x);
   float yDist = distance(mousePosition.y, texCoords.y);
   float dsqrd = dist*dist;

   if (mouseBtnDown == 1)
   {
      if (dist < brushSize)
         y = brushPower * (1.0 - dsqrd) * delta;
   }

   //Store the mask color in the blue channel
   //float maskColor = step(0.01, texture(mask_texture, texCoords).r); //In case of filtering, make only 0 or 1
   float maskColor = texture(mask_texture, texCoords).r; //In case of filtering, make only 0 or 1
   fragColor = vec4(0.0, y, maskColor, 1.0f);
}