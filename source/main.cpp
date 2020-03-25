
//------------------------------------------------------------------
//This file is organized in a stepwise manner to try and demonstrate
//how this water simulation may be integrated into an existing
//engine or project.
//------------------------------------------------------------------

#include "common.h"

bool windowOpen = true;

//Shaders
ShaderProgram drawingShader;
ShaderProgram imageShader;
ShaderProgram sumImageShader;
ShaderProgram waterPhysicsShader;
ShaderProgram waterSurfaceShader;
ShaderProgram shapeShader;
ShaderProgram flatShader;
ShaderProgram cubemapShader;

//Vertex arrays
unsigned int fullscreenVAO;
unsigned int waterBlockVAO;
unsigned int waterBlockElements;
unsigned int poolVAO;
int barrierCount = 0;
int barrierConfiguartion = 0;
unsigned int barrierVAOs[3];
unsigned int cubemapVAO;

//Framebuffers
unsigned int waterFBO; //For the textures used to run the water simulation (color, mask, and height)
unsigned int sceneFBO; //For the texture used to store scene information (scene, depth)

//Textures
Vector2u imageRes(128.0);
unsigned int colorTexture;
unsigned int maskTexture;
unsigned int heightTextures[2];
unsigned int surfaceDataTexture;
unsigned int sceneTexture;
unsigned int depthTexture;
unsigned int tileTexture;
unsigned int cubemapTexture;

//For calculating FPS
GLulong frameCount = 0;
sf::Clock fpsClock;
sf::Time fpsTime = fpsClock.getElapsedTime();
sf::Time previousTime = fpsTime;
int calculateFPS()
{
    frameCount++;

    //Get the number of milliseconds since start
    fpsTime = fpsClock.getElapsedTime();
    int timeInterval = fpsTime.asMilliseconds() - previousTime.asMilliseconds();

    if(timeInterval > 1000)
    {
        previousTime = fpsTime;
        frameCount = 0;
    }

    return frameCount / (timeInterval / 1000.0f);
}

void initGL()
{
    //Initialize glew
    GLenum error = glewInit();
    if (error != GLEW_OK)
        std::cout << "ERROR INITIALIZING GLEW!" << std::endl;

    //Setup OpenGL defaults
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    //Enable GL states
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Create a couple of new framebuffers for doing additional rendering.
    glGenFramebuffers(1, &waterFBO);
    glGenFramebuffers(1, &sceneFBO);

    //Set default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fetchGLErrors("Error initializing OpenGL:");
}

void initShaders()
{
    //-----------------------------------------------------
    //Build the shaders along with their samplers
    //-----------------------------------------------------
    //Shader for drawing the water
    ShaderInfo shader;
    shader.vShaderFile = "shaders/water_surface.vert";
    shader.fShaderFile = "shaders/water_surface.frag";
    waterSurfaceShader.programID = LoadShaders(shader);
    //Sampler
    waterSurfaceShader.setUniform("height_texture", 0);
    waterSurfaceShader.setUniform("surfaceData_texture", 1);
    waterSurfaceShader.setUniform("scene_texture", 2);
    waterSurfaceShader.setUniform("depth_texture", 3);
    waterSurfaceShader.setUniform("cubemap_texture", 4);

    //Shader for drawing color into a texture with the mouse
    shader.vShaderFile = "shaders/drawing_shader.vert";
    shader.fShaderFile = "shaders/drawing_shader.frag";
    drawingShader.programID = LoadShaders(shader);
    //Samplers
    drawingShader.setUniform("mask_texture", 0);

    //Shader for adding textures together
    shader.vShaderFile = "shaders/sum_image.vert";
    shader.fShaderFile = "shaders/sum_image.frag";
    sumImageShader.programID = LoadShaders(shader);
    //Samplers
    sumImageShader.setUniform("imageA_texture", 0);
    sumImageShader.setUniform("imageB_texture", 1);

    //Shader for displaying a texture image
    shader.vShaderFile = "shaders/image_shader.vert";
    shader.fShaderFile = "shaders/image_shader.frag";
    imageShader.programID = LoadShaders(shader);
    //Sampler
    imageShader.setUniform("color_texture", 0);

    //Shader for our water physics
    shader.vShaderFile = "shaders/water_physics.vert";
    shader.fShaderFile = "shaders/water_physics.frag";
    waterPhysicsShader.programID = LoadShaders(shader);
    //Samplers
    waterPhysicsShader.setUniform("height_texture", 0);
    waterPhysicsShader.setUniform("mask_texture", 1);

    //Shader for drawing our solid geometry
    shader.vShaderFile = "shaders/shape_shader.vert";
    shader.fShaderFile = "shaders/shape_shader.frag";
    shapeShader.programID = LoadShaders(shader);
    //Samplers
    shapeShader.setUniform("color_texture", 0);

    //Shader for our cubemap
    shader.vShaderFile = "shaders/cubemap.vert";
    shader.fShaderFile = "shaders/cubemap.frag";
    cubemapShader.programID = LoadShaders(shader);
    //Samplers
    cubemapShader.setUniform("cubemap_texture", 0);

    //Shader for drawing shapes into our mask texture
    shader.vShaderFile = "shaders/flat_shader.vert";
    shader.fShaderFile = "shaders/flat_shader.frag";
    flatShader.programID = LoadShaders(shader);
    //glUseProgram(flatShader);

    fetchGLErrors("Error in shader initialization:");
}

void initGeometry()
{
    //-----------------------------------------------------
    //Setup VAO for drawing textures to
    //-----------------------------------------------------
    glGenVertexArrays(1, &fullscreenVAO);
    glBindVertexArray(fullscreenVAO);

    float vertices[] = {
        -1.0, -1.0, 0.0,   0.0, 0.0,
        1.0, -1.0, 0.0,  1.0, 0.0,
        1.0, 1.0, 0.0,  1.0, 1.0,
        -1.0, 1.0, 0.0,  0.0, 1.0
    };
    GLushort indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int buffers[2];
    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (GLvoid*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (GLvoid*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //Create a plane for our water
    Vector2 planeSize(16.0, 16.0);
    Vector2 planeDensity(48.0, 48.0);
    newPlane(planeSize, planeDensity, &waterBlockVAO, &waterBlockElements);

    //Setup other geometry
    newCube(Vector3(0.0), Vector3(1.0), &cubemapVAO);
    newCube(Vector3(0.0, 5.0, 0.0), Vector3(8.0, 5, 8.0), &poolVAO, -1.0f); //Flip normals for this shape

    fetchGLErrors("Problem with geometry generation:");

    //-----------------------------------------------------
    //Textures
    //-----------------------------------------------------
    //texture2D("images/mask.png", GL_RGB, &maskTexture);
    texture2D(imageRes, GL_RGB16F, NULL, &colorTexture);
    texture2D(imageRes, GL_RGB, NULL, &maskTexture);
    texture2D(imageRes, GL_RGB32F, NULL, &heightTextures[0]);
    texture2D(imageRes, GL_RGB32F, NULL, &heightTextures[1]);
    texture2D(imageRes, GL_RGB16F, NULL, &surfaceDataTexture);
    texture2D("images/tile.png", GL_RGB, &tileTexture);
    textureCube("images/cubemap/park", &cubemapTexture);
    //We want these the same size as the window to prevent artifacts
    texture2D(Vector2(512.0, 600.0), GL_RGB, NULL, &sceneTexture);
    texture2D(Vector2(512.0, 600.0), GL_DEPTH_COMPONENT, NULL, &depthTexture);
    fetchGLErrors("Error generating textures:");
}

//Setup some example barrier configurations
void cycleBarriers()
{
    //Not the most efficient, but I didn't setup model matrices to alter so. . .
    glDeleteVertexArrays(barrierCount, barrierVAOs);

    barrierConfiguartion++;
    if (barrierConfiguartion > 3)
    {
        barrierCount = 0;
        barrierConfiguartion = 0;
        return;
    }

    switch (barrierConfiguartion)
    {
    case 1:
        {
            //One wall in the middle
            newCube(Vector3(0.0, 5.0, 0.0), Vector3(1, 5.0, 8.0), &barrierVAOs[0]);
            newCube(Vector3(0.5, 5.0, 0.0), Vector3(1, 5.0, 2.0), &barrierVAOs[1]);
            barrierCount = 2;
            break;
        }
    case 2:
        {
            //Two walls with a small space in between
            newCube(Vector3(0.0, 5.0, -4.5), Vector3(1, 5.0, 3.5), &barrierVAOs[0]);
            newCube(Vector3(0.0, 5.0, 4.5), Vector3(1, 5.0, 3.5), &barrierVAOs[1]);
            barrierCount = 2;
            break;
        }
    case 3:
        {
            //Three walls making up a zigzag
            newCube(Vector3(4.0, 5.0, 1.0), Vector3(1.0, 5.0, 7.0), &barrierVAOs[0]);
            newCube(Vector3(0.0, 5.0, -1.0), Vector3(1, 5.0, 7.0), &barrierVAOs[1]);
            newCube(Vector3(-4.0, 5.0, 1.0), Vector3(1.0, 5.0, 7.0), &barrierVAOs[2]);
            barrierCount = 3;
            break;
        }
    default:
        break;
    }
}

void bakeMaskTexture()
{
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);

    glViewport(0.0f, 0.0f, imageRes.x, imageRes.y);

    //Setup an orthographic view from above
    //Set to the dimensions of the water plane
    Matrix4 orthoProjMat = glm::ortho(-7.5f, 7.5f, -7.5f, 7.5f, 0.1f, 100.0f);
    Matrix4 orthoViewMat = glm::lookAt(Vector3(0.0f, 50.0f, 0.0f), Vector3(0.0f), Vector3(0.0f, 0.0f, -1.0f));

    //Draw the geometry
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, maskTexture, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    flatShader.setUniform("ModelViewProjection_mat", orthoProjMat*orthoViewMat*Matrix4(1.0));
    flatShader.enable();
    enableTexture2D(0, tileTexture);
    for (int i = 0; i < barrierCount; i++)
    {
        glBindVertexArray(barrierVAOs[i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
    disableTexture(0);

    glDeleteFramebuffers(1, &FBO);
    fetchGLErrors("Error baking barriers into mask texture:");
}

void drawScene(Vector3 cameraPos, Matrix4 viewMat, Matrix4 projectionMat)
{
    //Draw the skybox
    glDepthMask(GL_FALSE); //Draw this behind everything
    cubemapShader.setUniform("view_mat", glm::mat4(glm::mat3(viewMat)));
    cubemapShader.setUniform("projection_mat", projectionMat);
    cubemapShader.enable();
    enableTextureCube(0, cubemapTexture);
    glFrontFace(GL_CW); //Draw this cube's faces facing inward
    glBindVertexArray(cubemapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glFrontFace(GL_CCW);
    disableTexture(0);
    glDepthMask(GL_TRUE);
    fetchGLErrors("Error drawing skybox:");

        //Draw the pool
    shapeShader.setUniform("cameraPos", cameraPos);
    shapeShader.setUniform("ModelViewProjection_mat", projectionMat*viewMat*Matrix4(1.0));
    shapeShader.enable();
    enableTexture2D(0, tileTexture);
    glFrontFace(GL_CW); //Draw this cube's faces facing inward
    glBindVertexArray(poolVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glFrontFace(GL_CCW);
    fetchGLErrors("Error drawing pool geometry:");

    //Draw barrier geometry
    for (int i = 0; i < barrierCount; i++)
    {
        glBindVertexArray(barrierVAOs[i]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
    disableTexture(0);
    fetchGLErrors("Error drawing barrier geometry:");
}

int main()
{
    //Create context
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 1;
    settings.majorVersion = 4.4;
    settings.minorVersion = 3.1;
    sf::VideoMode vMode(1024, 600, 32);
    sf::RenderWindow window(vMode, "Water Block", sf::Style::Default, settings);

    //Initialize!
    initGL();
    initShaders();
    initGeometry();

    //Setup the text boxes we want for displaying helpful information
    //-------------------------------------------------------------------------------
    sf::Font font;
    if (!font.loadFromFile("OpenSans-Regular.ttf"))
        std::cout<<"Error loading font: OpenSans-Regular.tff"<<std::endl;
    sf::Text fpsTextbox("FPS: 0", font, 16);
    sf::Text loopCountTextbox("Physics Loops: 750", font, 16);
    sf::Text calcMSSecondTextbox("Physics Calc Time: 0", font, 16);
    sf::Text calcMSFrameTextbox("Physics Calc Time: 0", font, 16);
    fpsTextbox.setFillColor(sf::Color::Yellow);
    fpsTextbox.setPosition(5.0f, 5.0f);
    loopCountTextbox.setFillColor(sf::Color::Yellow);
    loopCountTextbox.setPosition(5.0f, 25.0f);
    calcMSSecondTextbox.setFillColor(sf::Color::Yellow);
    calcMSSecondTextbox.setPosition(5.0f, 45.0f);
    calcMSFrameTextbox.setFillColor(sf::Color::Yellow);
    calcMSFrameTextbox.setPosition(5.0f, 65.0f);
    unsigned int physicsLoops = 0;
    double physics_msPerSecond = 0;
    double physics_msPerFrame = 0;

    int infoIndex = 0;
    const int infoCount = 7;
    sf::Text infoString[infoCount];
    infoString[0] = sf::Text("Camera Distance: ", font, 14);
    infoString[1] = sf::Text("Brush Size     : ", font, 14);
    infoString[2] = sf::Text("Brush Power    : ", font, 14);
    infoString[3] = sf::Text("Fog Density    : ", font, 14);
    infoString[4] = sf::Text("Turbulence     : ", font, 14);
    infoString[5] = sf::Text("Refraction     : ", font, 14);
    infoString[6] = sf::Text("Reflection     : ", font, 14);

    //Have the original strings saved for later
    sf::String originString[] = {
        "Camera Distance: ", "Brush Size     : ", "Brush Power    : ",
        "Fog Density    : ", "Turbulence     : ", "Refraction     : ",
        "Reflection     : "
    };

    //Default values. cameraPosition is given to shapeShader, brush values to drawingShader,
    //and everything else to waterSurfaceShader.
    float infoValue[] = {
        30.0f, 0.15f, 25.0f, 0.75f, 0.0f, 0.25f, 0.35f
    };

    //How much will the scroll-wheel effect each value
    float infoValueOffset[] = {
        -2.0f, 0.01f, 2.5f, 0.05f, 0.025f, 0.05f, 0.05f
    };

    float lineSpace = 5.0f;
    for (int i = 0; i < infoCount; i++)
    {
        infoString[i].setFillColor(sf::Color::White);
        infoString[i].setPosition(520.0f, lineSpace);
        lineSpace += 15.0f;
    }
    //-------------------------------------------------------------------------------

    //Initialize the clock
    sf::Clock deltaClock;
    sf::Clock secondClock;
    double delta = 0.0;

    //750 water physics loops per second
    const double physics_dt = 1.0 / 750.0;
    double accumulator = 0.0;

    //Setup our 3D view
    Vector2u currentMousePos, lastMousePos = Vector2(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
    float cameraDistance = 30.0f;
    Vector3 cameraPosition = Vector3(0.0f, 8.0f, cameraDistance);
    Vector3 viewCenter(0.0, 4.0, 0.0);
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    Matrix4 modelMatrix = Matrix4(1.0);
    Matrix4 projectionMatrix = glm::perspective(glm::radians(45.0f), 512.0f / 600.0f, 0.1f, 100.0f);
    Matrix4 viewMatrix = glm::lookAt(cameraPosition, viewCenter, Vector3(0.0, 1.0, 0.0));

    //Index for ping-ponging textures
    GLushort currentTexture = 0;
    GLushort nextTexture = 1;

    //Do we need to update the barrier mask texture?
    bool updateMask = true;

    //User input
    bool rightMouseDown = false;

    while (windowOpen)
    {
        //Delta
        delta = deltaClock.getElapsedTime().asSeconds();
        deltaClock.restart();

        //Update FPS and text
        //---------------------------------------------------------
        int fps = calculateFPS();
        if (secondClock.getElapsedTime().asMilliseconds() > 999)
        {
            std::stringstream ss;
            ss << fps;
            sf::String textString = ss.str();
            fpsTextbox.setString("FPS: " + textString);

            ss.str("");
            ss << physicsLoops;
            textString = ss.str();
            loopCountTextbox.setString("Physics Loops: " + textString);

            ss.str("");
            ss << physics_msPerSecond;
            textString = ss.str();
            calcMSSecondTextbox.setString("Physics Calc Time: " + textString + "ms/s");

            ss.str("");
            ss << physics_msPerFrame;
            textString = ss.str();
            calcMSFrameTextbox.setString("Physics Calc Time: " + textString + "ms/frame");

            secondClock.restart();
            physicsLoops = 0;
            physics_msPerSecond = 0.0;
            physics_msPerFrame = 0.0;
        }

        for (int i = 0; i < infoCount; i++)
        {
            std::stringstream ss;
            ss << infoValue[i];
            sf::String valueString = ss.str();
            infoString[i].setString(originString[i] + valueString);

            //Highlight which field is selected
            if (i == infoIndex)
                infoString[i].setFillColor(sf::Color::Yellow);
            else
                infoString[i].setFillColor(sf::Color::White);
        }
        //---------------------------------------------------------

        //Bake all barrier objects into the mask texture
        if (updateMask)
        {
            bakeMaskTexture();
            updateMask = false;
        }

        //Use some mouse info as uniforms so we can draw to a texture
        glViewport(0.0f, 0.0f, imageRes.x, imageRes.y);
        currentMousePos = Vector2(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
        float mouseX = (float)(sf::Mouse::getPosition(window).x/512.0);
        float mouseY = 1.0f - (float)(sf::Mouse::getPosition(window).y/600.0);

        drawingShader.setUniform("mousePosition", Vector2(mouseX, mouseY));
        drawingShader.setUniform("delta", (float)delta);

        rotationX = 0.0;
        rotationY = 0.0;

        //Handle input
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                {
                    windowOpen = false;
                    break;
                }
            case sf::Event::KeyPressed:
                {
                    if (event.key.code == sf::Keyboard::Space)
                    {
                        //Cycle through different barrier configurations
                        cycleBarriers();
                        updateMask = true;
                    }
                    if (event.key.code == sf::Keyboard::Z)
                    {
                        //Turn off barriers
                        barrierConfiguartion = 100; //Just make it big in case I add more later
                        cycleBarriers();
                        updateMask = true;
                    }
                    if (event.key.code == sf::Keyboard::Left)
                    {
                        //Allow the user to use either the arrow keys OR the mouse-wheel to adjust values
                        infoValue[infoIndex] -= infoValueOffset[infoIndex];

                        //Bush power is the only value where we want to allow negatives
                        if (infoIndex != 2)
                        {
                            if (infoValue[infoIndex] < infoValueOffset[infoIndex])
                                infoValue[infoIndex] = 0.0f;
                        }
                    }
                    if (event.key.code == sf::Keyboard::Right)
                    {
                        infoValue[infoIndex] += infoValueOffset[infoIndex];

                        //Bush power is the only value where we want to allow negatives
                        if (infoIndex != 2)
                        {
                            if (infoValue[infoIndex] < infoValueOffset[infoIndex])
                                infoValue[infoIndex] = 0.0f;
                        }
                    }
                    if (event.key.code == sf::Keyboard::Up)
                    {
                        //Switch through what we want to change
                        infoIndex--;
                        if (infoIndex < 0)
                            infoIndex = infoCount - 1;
                    }
                    if (event.key.code == sf::Keyboard::Down)
                    {
                        //Switch through what we want to change
                        infoIndex++;
                        if (infoIndex >= infoCount)
                            infoIndex = 0;
                    }
                    break;
                }
            case sf::Event::MouseWheelScrolled:
                {
                    //Adjust the selected infoValue by infoValueOffset
                    infoValue[infoIndex] += infoValueOffset[infoIndex] * event.mouseWheelScroll.delta;

                    //Bush power is the only value where we want to allow negatives
                    if (infoIndex != 2)
                    {
                        if (infoValue[infoIndex] < infoValueOffset[infoIndex])
                            infoValue[infoIndex] = 0.0f;
                    }

                    break;
                }
            case sf::Event::MouseMoved:
                {
                    if (rightMouseDown)
                    {
                        rotationX = ((float)currentMousePos.x - (float)lastMousePos.x) * 0.5;
                        rotationY = ((float)currentMousePos.y - (float)lastMousePos.y) * 0.5;
                        if (cameraPosition.y < 0.0f)
                            cameraPosition.y = 0.0f;
                        lastMousePos = currentMousePos;
                    }
                    break;
                }
            case sf::Event::MouseButtonPressed:
                {
                    if (event.mouseButton.button == sf::Mouse::Left)
                    {
                        drawingShader.setUniform("mouseBtnDown", 1);
                    }
                    if (event.mouseButton.button == sf::Mouse::Right)
                    {
                        rightMouseDown = true;
                        lastMousePos = currentMousePos;
                    }
                    break;
                }
            case sf::Event::MouseButtonReleased:
                {
                    rightMouseDown = false;
                    drawingShader.setUniform("mouseBtnDown", 0);
                    break;
                }
            default:
                break;
            }
        }

        //Just because I want to scroll down to zoom out
        if (infoValue[0] < -infoValueOffset[0])
            infoValue[0] = 0.0f;

        //Update our camera view
        //---------------------------------------------------
        float deltaDistance = infoValue[0] - cameraDistance;
        Vector3 viewVector = glm::normalize(cameraPosition - Vector3(0.0f));
        cameraPosition += viewVector * deltaDistance;
        cameraDistance = infoValue[0];
        Vector3 rightVector = glm::normalize(glm::cross(Vector3(0.0f, 1.0f, 0.0f), viewVector));
        Vector3 upVector = glm::normalize(glm::cross(rightVector, viewVector));
        cameraPosition = glm::rotate(cameraPosition, glm::radians(rotationX), upVector);
        cameraPosition = glm::rotate(cameraPosition, glm::radians(rotationY), rightVector);
        viewMatrix = glm::lookAt(cameraPosition, viewCenter, Vector3(0.0, 1.0, 0.0));
        Matrix4 uniformMatrix = projectionMatrix * viewMatrix * modelMatrix;
        //---------------------------------------------------

        //Setup for drawing into an intermediate color texture. Since we reuse this Framebuffer Object,
        //make sure the second attachment is reset back to none (GL_NONE).
        GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_NONE };
        glBindFramebuffer(GL_FRAMEBUFFER, waterFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
        glDrawBuffers(2, attachments);

        //Draw our mouse painting. The contents of maskTexture are stored in colorTexture's blue
        //channel, and later copied into a height texture which cuts down on additional sampling
        //in the physics shader.
        glClear(GL_COLOR_BUFFER_BIT);
        drawingShader.setUniform("brushSize", infoValue[1]);
        drawingShader.setUniform("brushPower", infoValue[2]);
        drawingShader.enable();
        enableTexture2D(0, maskTexture);
        glBindVertexArray(fullscreenVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        //glBindVertexArray(0); //Leave fullscreenVAO bound until after the physics loop is done with it
        disableTexture(0);
        fetchGLErrors("Error after drawing stage:");

        //Add our drawing changes into the desired height texture
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, heightTextures[1], 0);
        glClear(GL_COLOR_BUFFER_BIT);
        sumImageShader.enable();
        enableTexture2D(0, colorTexture);
        enableTexture2D(1, heightTextures[0]);
        //glBindVertexArray(fullscreenVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        //glBindVertexArray(0);
        disableTexture(1);
        disableTexture(0);
        fetchGLErrors("Problem copying data into height texture:");

        //--------------------------------------------------------
        //Water physics loop
        //--------------------------------------------------------
        //I opted for a fixed simulation timestep as described here:
        //"Fix Your Timestep! Gaffer On Games"
        //https://gafferongames.com/post/fix_your_timestep
        //---
        //Although this will keep the simulation running fairly consistently between different
        //machines, I did not implement state interpolation. This may lead to visual
        //stutters at a lower number of physics loops, even though we're running at
        //hundreds/thousands of frames per second.
        unsigned int physicsStartTime = deltaClock.getElapsedTime().asMicroseconds();

        accumulator += delta;

        //Second target for rendering
        attachments[1] = GL_COLOR_ATTACHMENT1;

        while (accumulator >= physics_dt)
        {
            //Run our water physics
            waterPhysicsShader.enable();
            enableTexture2D(0, heightTextures[currentTexture]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, heightTextures[nextTexture], 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, surfaceDataTexture, 0);
            glDrawBuffers(2, attachments);
            glClear(GL_COLOR_BUFFER_BIT);
            //glBindVertexArray(fullscreenVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            //glBindVertexArray(0);
            disableTexture(1);
            disableTexture(0);

            accumulator -= physics_dt;

            //Ping-pong textures
            currentTexture++;
            nextTexture++;
            if (currentTexture > 1)
                currentTexture = 0;
            if (nextTexture > 1)
                nextTexture = 0;

            physicsLoops++;

            fetchGLErrors("Error in physics loop:");
        }
        glBindVertexArray(0);

        //Calculate the time it took for the physics step as both ms/frame, and total ms taken out of a second.
        physics_msPerFrame = (deltaClock.getElapsedTime().asMicroseconds() - physicsStartTime) / 1000.0;
        physics_msPerSecond += physics_msPerFrame;
        //--------------------------------------------------------
        //--------------------------------------------------------
        //--------------------------------------------------------

        //Render the scene into "sceneTexture", and capture scene depth into "depthTexture"
        //--------------------------------------------------------
        //In this example, "sceneTexture" has all of the objects in the scene (or at least the
        //ones that could be seen underwater) rendered into it while also saving the zBuffer contents.
        //This texture is passed to the waterSurfaceShader where it, along with depthTexture, are
        //used to create the visual surface effects.
        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
        glViewport(0.0, 0.0, 512.0, 600.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        fetchGLErrors("Problem setting up framebuffer for scene render:");

        drawScene(cameraPosition, viewMatrix, projectionMatrix);
        //--------------------------------------------------------

        //-----------------------------------------------------
        //View Output
        //-----------------------------------------------------

        //Display desired texture preview on the left side of the screen.  .  .
        glBindFramebuffer(GL_FRAMEBUFFER, 0); //Default framebuffer
        glViewport(0.0, 0.0, 512.0, 600.0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        imageShader.enable();
        enableTexture2D(0, heightTextures[0]);
        glBindVertexArray(fullscreenVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        disableTexture(0);
        fetchGLErrors("Problem drawing texture preview:");

        //. . . and draw the 3D results on the right
        glViewport(512.0, 0.0, 512.0, 600.0);
        drawScene(cameraPosition, viewMatrix, projectionMatrix);

        //Draw our WaterBlock. The heightmap texture is used in the vertex shader to alter the
        //geometry. The other 4 are used in the fragment shader for extra visual juiciness.
        //-----------------------------------------------------------------
        waterSurfaceShader.setUniform("cameraPos", cameraPosition);
        waterSurfaceShader.setUniform("ModelViewProjection_mat", uniformMatrix);
        waterSurfaceShader.setUniform("fogDensity", infoValue[3]);
        waterSurfaceShader.setUniform("turbulenceStrength", infoValue[4]);
        waterSurfaceShader.setUniform("refractionStrength", infoValue[5]);
        waterSurfaceShader.setUniform("reflectionStrength", infoValue[6]);
        waterSurfaceShader.enable();
        enableTexture2D(0, heightTextures[0]);
        enableTexture2D(1, surfaceDataTexture);
        enableTexture2D(2, sceneTexture);
        enableTexture2D(3, depthTexture);
        enableTextureCube(4, cubemapTexture);
        //glDisable(GL_CULL_FACE); //Double-sided water surface
        glBindVertexArray(waterBlockVAO);
        glDrawElements(GL_TRIANGLES, waterBlockElements, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        //glEnable(GL_CULL_FACE);
        disableTexture(4);
        disableTexture(3);
        disableTexture(2);
        disableTexture(1);
        disableTexture(0);
        fetchGLErrors("Error drawing water:");
        //-----------------------------------------------------
        //-----------------------------------------------------
        //-----------------------------------------------------

        //Draw text boxes
        window.pushGLStates();
        window.draw(fpsTextbox);
        window.draw(loopCountTextbox);
        window.draw(calcMSSecondTextbox);
        window.draw(calcMSFrameTextbox);
        for (int i = 0; i < infoCount; i++)
            window.draw(infoString[i]);
        window.popGLStates();

        //Swap buffers and display
        //glFlush();
        window.display();
        fetchGLErrors("Error with final display:");

    }

    //Cleanup a bit
    glDeleteBuffers(1, &waterFBO);
    glDeleteBuffers(1, &sceneFBO);
    glDeleteVertexArrays(1, &fullscreenVAO);
    glDeleteVertexArrays(1, &waterBlockVAO);
    glDeleteVertexArrays(1, &poolVAO);
    glDeleteVertexArrays(1, &cubemapVAO);
    glDeleteVertexArrays(barrierCount, barrierVAOs);
    glDeleteTextures(1, &maskTexture);
    glDeleteTextures(1, &colorTexture);
    glDeleteTextures(2, heightTextures);
    glDeleteTextures(1, &surfaceDataTexture);
    glDeleteTextures(1, &sceneTexture);
    glDeleteTextures(1, &depthTexture);
    glDeleteTextures(1, &tileTexture);
    glDeleteTextures(1, &cubemapTexture);

    //Get any last errors before closing out
    if (fetchGLErrors("Error after cleanup:"))
    {
        //Pause so we can see it
        int in;
        std::cin>>in;
    }

    return 0;
}
