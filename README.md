# 3DWater-Simulation
Real time 3D water simulation on the GPU

This is intended to be a viable solution for people looking for fast, real time water physics in their projects. It uses SFML for context creation, user input, and text display, but does not rely on it for any implementation of the simulation itself. The hope is that the code in this standalone executable are easily adapted to existing projects without in depth prerequisite knowledge of SFML or OpenGL.

Due to some oddities with the compiler I used, libgcc_s_sjlj-1.dll and libstdc++-6.dll need to be in the root folder in order to run WaterBlock.exe. I'll switch compilers in the future.

Special thanks to:

FabooGuy for use of the tile texture
https://www.deviantart.com/fabooguy/art/Marble-Floor-Tiles-Texture-Tileable-2048x2048-436170199

Emil Persson, aka Humus for use of the park cube map texture
http://www.humus.name
