#ifndef _COMMON_H_
#define _COMMON_H_

#define GLEW_STATIC

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>

typedef glm::vec2 Vector2;
typedef glm::uvec2 Vector2u;
typedef glm::vec3 Vector3;
typedef glm::mat4x4 Matrix4;

//Textures
void texture2D(Vector2u size, int format, const void* pixelData, unsigned int *glTexture);
bool texture2D(const char* imageName, int format, unsigned int *glTexture);
void textureCube(std::string imageName, unsigned int *glTexture);
void enableTexture2D(unsigned int textureUnit, unsigned int textureID);
void enableTextureCube(unsigned int textureUnit, unsigned int textureID);
void disableTexture(unsigned int textureUnit);

//Shaders
struct ShaderInfo
{
	GLenum vTarget;
	const char *vShaderFile;
	GLenum fTarget;
	const char *fShaderFile;
};

struct ShaderProgram
{
    unsigned int programID;
    bool active = false;
    void enable();
    void disable();
    void setUniform(const char *attributeName, int value);
    void setUniform(const char *attributeName, float value);
    void setUniform(const char *attributeName, Vector2 vec);
    void setUniform(const char *attributeName, Vector3 vec);
    void setUniform(const char *attributeName, Matrix4 matrix);
};

unsigned int LoadShaders(ShaderInfo shaderInfo);
const char* getShaderProgram(const char *filePath, std::string &shaderProgramText);

//Geometry
void newCube(Vector3 position, Vector3 dimensions, unsigned int *VAO, float n = 1.0f);
void newPlane(Vector2 dimensions, Vector2 density, unsigned int *VAO, unsigned int *elements);

//Errors
bool fetchGLErrors(const char *message);

#endif // _COMMON_H_
