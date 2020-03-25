
//Much of this file is dedicated to abstracting out OpenGL so
//that main.cpp can be focused on example program flow.

#include "common.h"

//-----------------------------------------------------------------
//Texture creation
//-----------------------------------------------------------------
void texture2D(Vector2u size, int format, const void* pixelData, unsigned int *glTexture)
{
    glGenTextures(1, glTexture);
    glBindTexture(GL_TEXTURE_2D, *glTexture);

    if (format == GL_DEPTH_COMPONENT)
        glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixelData);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);

    //Set the wrapping options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    //Set the filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //In theory, GL_NEAREST should be "faster"
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool texture2D(const char* imageName, int format, unsigned int *glTexture)
{
    //Load the image
    sf::Image image;
    if (!image.loadFromFile(imageName))
    {
        std::cout << "Failed to Load Image: " << imageName << std::endl;
        return false;
    }

    int width = image.getSize().x;
    int height = image.getSize().y;

    glGenTextures(1, glTexture);
    glBindTexture(GL_TEXTURE_2D, *glTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
    glGenerateMipmap(GL_TEXTURE_2D);

    //Set the texture wrapping options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Set the texture filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

//aka cubemap
void textureCube(std::string imageName, unsigned int *glTexture)
{
    glGenTextures(1, glTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *glTexture);

    std::string suffix[6] = {
        "_right.png",
        "_left.png",
        "_top.png",
        "_bottom.png",
        "_front.png",
        "_back.png"
    };

    for(unsigned int i = 0; i < 6; i++)
    {
        std::string fileName = imageName + suffix[i];
        sf::Image image;
        if (!image.loadFromFile(fileName.c_str()))
        {
            std::cout << "Failed to Load Image: " << imageName << std::endl;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            continue;
        }

        int width = image.getSize().x;
        int height = image.getSize().y;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void enableTexture2D(unsigned int textureUnit, unsigned int textureID)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void enableTextureCube(unsigned int textureUnit, unsigned int textureID)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void disableTexture(unsigned int textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    //Just unbind everything
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

//-----------------------------------------------------------------
//Shader creation and loading
//-----------------------------------------------------------------
const char* getShaderProgram(const char *filePath, std::string &shader)
{
	std::fstream shaderFile(filePath, std::ios::in);

	if (shaderFile.is_open())
	{
		std::stringstream buffer;
		buffer << shaderFile.rdbuf();
		shader = buffer.str();
		buffer.clear();
	}
	shaderFile.close();

	return shader.c_str();
}

unsigned int LoadShaders(ShaderInfo shaderInfo)
{
	unsigned int program;
	unsigned int vertexShader;
	unsigned int fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER); //create a vertex shader object
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //create a fragment shader object

	//Load and compile vertex shader
	std::string shaderProgramText;
	const char* text = getShaderProgram(shaderInfo.vShaderFile, shaderProgramText);
	glShaderSource(vertexShader, 1, &text, NULL);
	glCompileShader(vertexShader);

	int status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
		std::cerr << "\nVertex Shader '" << shaderInfo.vShaderFile << "' compilation failed..." << '\n';

    //Get errors from the vertex shader
    GLsizei length;
    GLsizei bufferSize = 200;
	std::vector<char> errorLog(bufferSize);
	glGetShaderInfoLog(vertexShader, bufferSize, &length, &errorLog[0]);
	for (int i = 0; i < length; i++)
		std::cout << errorLog[i];

	//Load and compile fragment shader
	shaderProgramText = "";
	text = getShaderProgram(shaderInfo.fShaderFile, shaderProgramText);
	glShaderSource(fragmentShader, 1, &text, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
		std::cerr << "\nFragment Shader '" << shaderInfo.fShaderFile << "' compilation failed..." << '\n';

    //Get errors from the fragment shader
	glGetShaderInfoLog(fragmentShader, bufferSize, &length, &errorLog[0]);
	for (int i = 0; i < length; i++)
		std::cout << errorLog[i];

	//Create the shader program
	program = glCreateProgram();

	//Attach the shaders to program
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	//Link the objects for an executable program
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
		std::cout << "Link failed..." << std::endl;

    //Cleanup shaders
    glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// return the program
	return program;
}

void ShaderProgram::enable()
{
    active = true;
    glUseProgram(programID);
}

void ShaderProgram::disable()
{
    active = false;
    glUseProgram(0);
}

void ShaderProgram::setUniform(const char *attributeName, int value)
{
    //if (!active)
        enable();

    unsigned int uniform_loc = glGetUniformLocation(programID, attributeName);
    glUniform1i(uniform_loc, value);
    //disable();
}

void ShaderProgram::setUniform(const char *attributeName, float value)
{
    //if (!active)
        enable();

    unsigned int uniform_loc = glGetUniformLocation(programID, attributeName);
    glUniform1f(uniform_loc, value);
    //disable();
}

void ShaderProgram::setUniform(const char *attributeName, Vector2 vec)
{
    //if (!active)
        enable();

    unsigned int uniform_loc = glGetUniformLocation(programID, attributeName);
    glUniform2f(uniform_loc, (float)vec.x, (float)vec.y);
    //disable();
}

void ShaderProgram::setUniform(const char *attributeName, Vector3 vec)
{
    //if (!active)
        enable();

    unsigned int uniform_loc = glGetUniformLocation(programID, attributeName);
    glUniform3f(uniform_loc, (float)vec.x, (float)vec.y, (float)vec.z);
    //disable();
}

void ShaderProgram::setUniform(const char *attributeName, Matrix4 matrix)
{
    //if (!active)
        enable();

    unsigned int uniform_loc = glGetUniformLocation(programID, attributeName);
    glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(matrix));
    //disable();
}

//-----------------------------------------------------------------
//Geometry creation
//-----------------------------------------------------------------

//Cube- Good heavens the hackery! Why am I not using a model matrix?
void newCube(Vector3 position, Vector3 size, unsigned int *VAO, float n)
{
    //Cubemap geometry

    //Define the vertices and texture coordinates of the shape
    float vertices[] = {
        //Vertices              //Normals             //Texture coordinates
        -1.0f,  1.0f, 1.0f,      0.0f, 0.0f, 1.0f*n,     0.0f, 1.0f*size.y,
        -1.0f, -1.0f, 1.0f,      0.0f, 0.0f, 1.0f*n,     0.0f, 0.0f,
        1.0f, -1.0f, 1.0f,       0.0f, 0.0f, 1.0f*n,     1.0f*size.x, 0.0f,
        1.0f, -1.0f, 1.0f,       0.0f, 0.0f, 1.0f*n,     1.0f*size.x, 0.0f,
        1.0f,  1.0f, 1.0f,       0.0f, 0.0f, 1.0f*n,     1.0f*size.x, 1.0f*size.y,
        -1.0f,  1.0f, 1.0f,      0.0f, 0.0f, 1.0f*n,     0.0f, 1.0f*size.y,

        1.0f, -1.0f,  1.0f,      1.0f*n, 0.0f, 0.0f,     0.0f, 1.0f*size.z,
        1.0f, -1.0f, -1.0f,      1.0f*n, 0.0f, 0.0f,     0.0f, 0.0f,
        1.0f,  1.0f, -1.0f,      1.0f*n, 0.0f, 0.0f,     1.0f*size.y, 0.0f,
        1.0f,  1.0f, -1.0f,      1.0f*n, 0.0f, 0.0f,     1.0f*size.y, 0.0f,
        1.0f,  1.0f,  1.0f,      1.0f*n, 0.0f, 0.0f,     1.0f*size.y, 1.0f*size.z,
        1.0f, -1.0f,  1.0f,      1.0f*n, 0.0f, 0.0f,     0.0f, 1.0f*size.z,

        -1.0f, -1.0f, -1.0f,     -1.0f*n, 0.0f, 0.0f,     0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,     -1.0f*n, 0.0f, 0.0f,     0.0f, 1.0f*size.y,
        -1.0f,  1.0f,  1.0f,     -1.0f*n, 0.0f, 0.0f,     1.0f*size.y, 1.0f*size.y,
        -1.0f,  1.0f,  1.0f,     -1.0f*n, 0.0f, 0.0f,     1.0f*size.y, 1.0f*size.y,
        -1.0f,  1.0f, -1.0f,     -1.0f*n, 0.0f, 0.0f,     1.0f*size.y, 0.0f,
        -1.0f, -1.0f, -1.0f,     -1.0f*n, 0.0f, 0.0f,     0.0f, 0.0f,

        -1.0f, -1.0f,  -1.0f,    0.0f, 0.0f, -1.0f*n,     0.0f, 0.0f,
        -1.0f,  1.0f,  -1.0f,    0.0f, 0.0f, -1.0f*n,     0.0f, 1.0f*size.y,
        1.0f,  1.0f,  -1.0f,     0.0f, 0.0f, -1.0f*n,     1.0f*size.x, 1.0f*size.y,
        1.0f,  1.0f,  -1.0f,     0.0f, 0.0f, -1.0f*n,     1.0f*size.x, 1.0f*size.y,
        1.0f, -1.0f,  -1.0f,     0.0f, 0.0f, -1.0f*n,     1.0f*size.x, 0.0f,
        -1.0f, -1.0f,  -1.0f,    0.0f, 0.0f, -1.0f*n,     0.0f, 0.0f,

        -1.0f,  -1.0f, -1.0f,    0.0f, -1.0f*n, 0.0f,     0.0f, 0.0f,
        1.0f,  -1.0f, -1.0f,     0.0f, -1.0f*n, 0.0f,     1.0f*size.x, 0.0f,
        1.0f,  -1.0f,  1.0f,     0.0f, -1.0f*n, 0.0f,     1.0f*size.x, 1.0f*size.z,
        1.0f,  -1.0f,  1.0f,     0.0f, -1.0f*n, 0.0f,     1.0f*size.x, 1.0f*size.z,
        -1.0f,  -1.0f,  1.0f,    0.0f, -1.0f*n, 0.0f,     0.0f, 1.0f*size.z,
        -1.0f,  -1.0f, -1.0f,    0.0f, -1.0f*n, 0.0f,     0.0f, 0.0f,

        -1.0f, 1.0f, -1.0f,      0.0f, 1.0f*n, 0.0f,    0.0f, 0.0f,
        -1.0f, 1.0f,  1.0f,      0.0f, 1.0f*n, 0.0f,    0.0f, 1.0f*size.z,
        1.0f, 1.0f, -1.0f,       0.0f, 1.0f*n, 0.0f,    1.0f*size.x, 0.0f,
        1.0f, 1.0f, -1.0f,       0.0f, 1.0f*n, 0.0f,    1.0f*size.x, 0.0f,
        -1.0f, 1.0f,  1.0f,      0.0f, 1.0f*n, 0.0f,    0.0f, 1.0f*size.z,
        1.0f, 1.0f,  1.0f,       0.0f, 1.0f*n, 0.0f,    1.0f*size.x, 1.0f*size.z
    };

    //Adjust the size and position of the vertices. Again, why not a model matrix?
    //I'm in to deep now.
    for (int i = 0; i < 288; i+= 8)
    {
        vertices[i] *= size.x;
        vertices[i+1] *= size.y;
        vertices[i+2] *= size.z;
        vertices[i] += position.x;
        vertices[i+1] += position.y;
        vertices[i+2] += position.z;
    }

    //
    glGenVertexArrays(1, VAO);
    glBindVertexArray(*VAO);

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(0));
    glEnableVertexAttribArray(0);
    //Normal data
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //UV data
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//Generate a plane with the desired size and quad density
void newPlane(Vector2 dimensions, Vector2 density, unsigned int *VAO, unsigned int *elements)
{
    std::vector<Vector3> vertices;
    std::vector<Vector2> texCoords;
    std::vector<GLushort> indices;

    //Density = quad density
    Vector2 offset = dimensions / density;

    //Fill with vertices
    Vector3 vertexPos = Vector3(dimensions.x, 0.0f, dimensions.y) / Vector3(-2.0f, 1.0f, -2.0f);
    for (int z = 0; z < (int)density.y + 1; z++)
    {
        float zPos = vertexPos.z + (offset.y * z);
        for (int x = 0; x < (int)density.x + 1; x++)
        {
            float xPos = vertexPos.x + (offset.x * x);
            vertices.push_back(Vector3(xPos, 0.0f, zPos));
        }
    }

    //Fill with texture coordinates
    offset = Vector2(1.0f) / density;
    Vector2 texPos = Vector2(0.0f);
    for (int y = 0; y < (int)density.y + 1; y++)
    {
        //Reverse the y coordinate just so our drawing matches the 3D preview
        float yPos = 1.0f - (texPos.y + (offset.y * y));
        for (int x = 0; x < (int)density.x + 1; x++)
        {
            float xPos = texPos.x + (offset.x * x);
            texCoords.push_back(Vector2(xPos, yPos));
        }
    }

    //Fill with indices
    GLushort a = 0;
    GLushort column = 0;
    GLushort maxQuads = (GLushort)density.x * (GLushort)density.y;
    GLushort indexOffset = (GLushort)density.x + 1;
    for (GLushort i = 0; i < maxQuads; i++)
    {
        GLushort d = a + 1;
        GLushort b = a + indexOffset;
        GLushort c = d + indexOffset;

        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
        indices.push_back(c);
        indices.push_back(d);
        indices.push_back(a);

        //Jump up the next row
        a++;
        column++;
        if (column == (GLushort)density.x)
        {
            a++;
            column = 0;
        }
    }


    //Pack our plane data into our vertex array object
    *elements = indices.size();
    std::size_t vertices_size = vertices.size() * sizeof(Vector3);
    std::size_t texCoords_size = texCoords.size() * sizeof(Vector2);

    glGenVertexArrays(1, VAO);
    glBindVertexArray(*VAO);

    unsigned int buffers[2];
    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, (vertices_size + texCoords_size), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, &vertices[0]);
    glBufferSubData(GL_ARRAY_BUFFER, vertices_size, texCoords_size, &texCoords[0]);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (GLvoid*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), (GLvoid*)(vertices_size));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, *elements*sizeof(GLushort), &indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//-----------------------------------------------------------------
//OpenGL Errors
//-----------------------------------------------------------------
//Not very robust, but scattering these around should help narrow down problems
bool fetchGLErrors(const char *message)
{
    bool thrownError = false;
    std::string errorString;
    GLenum errorCode = glGetError();

    //Keep going until all error have been printed
    while (errorCode != 0)
    {
        thrownError = true;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:
            {
                errorString = "INVALID_ENUM";
                break;
            }
        case GL_INVALID_VALUE:
            {
                errorString = "INVALID_VALUE";
                break;
            }
        case GL_INVALID_OPERATION:
            {
                errorString = "INVALID_OPERATION";
                break;
            }
        case GL_STACK_OVERFLOW:
            {
                errorString = "STACK_OVERFLOW";
                break;
            }
        case GL_STACK_UNDERFLOW:
            {
                errorString = "STACK_OVERFLOW";
                break;
            }
        case GL_OUT_OF_MEMORY:
            {
                errorString = "OUT_OF_MEMORY";
                break;
            }
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            {
                errorString = "INVALID_FRAMEBUFFER_OPERATION";
                break;
            }
        }

        std::cout<<message<<" "<<errorString<<std::endl;
        errorCode = glGetError();
    }

    return thrownError;
}
