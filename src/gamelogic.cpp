#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "textures.hpp"
#include "glm/ext.hpp"
#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

const int NO_OF_LIGHT_SOURCES = 1;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* floorNode;
SceneNode* tower1Node;
SceneNode* light1Node;
SceneNode* light2Node;
SceneNode* light3Node;
SceneNode* ballNode;

SceneNode* floor1Node;
SceneNode* floor2Node;
SceneNode* floor3Node;
SceneNode* floor4Node;

const glm::vec2 floor_span = glm::vec2(400.0f, 400.0f);
const glm::vec3 floor_position = glm::vec3(-200.0f, -50.0f, 200.0f);

const glm::vec3 cube1_position = glm::vec3(0.0f, -45.0f, 40.0f);
const glm::vec3 cube2_position = glm::vec3(40.0f, -45.0f, 0.0f);
const glm::vec3 cube3_position = glm::vec3(-40.0f, -45.0f, 0.0f);
const glm::vec3 cube4_position = glm::vec3(0.0f, -45.0f, -40.0f);

const glm::vec3 tower_dimensions = glm::vec3(5.0f, 60.0f, 5.0f);
const glm::vec3 tower_position = glm::vec3(0.0f, -80.0f, 0.0f);

const glm::vec3 ball_position = glm::vec3(0.0f, 62.5f, 0.0f);
const float ball_radius = 1.0f;

const glm::vec3 light1_position = glm::vec3(0.0f, 62.5f, 0.0f);
const glm::vec3 light1_pointed_angle = glm::vec3(1.0f, 0.0f, 0.0f);

double pi_value = 0.3;

glm::vec4 lightArray[NO_OF_LIGHT_SOURCES];

// These are heap allocated, because they should not be initialised at the start of the program
Gloom::Shader* worldShader;
Gloom::Shader* shadowShader;


sf::Sound* sound;
sf::SoundBuffer* buffer;

CommandLineOptions options;


//Game logic
bool hasStarted = false;
bool hasLost = false;
bool jumpedToNextFrame = false;
bool isPaused = false;

//Camera Control
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 upAlignment = glm::vec3(0.0f, 1.0f, 0.0f);

//Uniforms
glm::mat4 VP;

//Shadow Configs
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
GLuint shadowMap;
unsigned int depthMapFBO;
glm::mat4 lightSpaceMatrix;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

//Mouse logic
bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

//Mouse Control
double mouseSensitivity = 1.0f;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
float yaw = -90.0f;
float pitch = 0.0f;


void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    lastMouseX = x;
    lastMouseY = y;

    float sensitivity = 0.07f;
    deltaX *= sensitivity;
    deltaY *= sensitivity;

    yaw += deltaX;
    pitch += deltaY;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = - 89.0f;
    }

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = -sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    printf("Game init started\n");

    options = gameOptions;


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    printf("Creating shaders\n");
    worldShader = new Gloom::Shader();
    worldShader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    worldShader->activate();

    shadowShader = new Gloom::Shader();
    shadowShader->makeBasicShader("../res/shaders/shadow.vert", "../res/shaders/shadow.frag");

    setUpShadows();
    
    printf("Constructing SceneNodes\n");
    // Construct scene
    rootNode = createSceneNode();
    floorNode = createSceneNode();
    tower1Node = createSceneNode();
    ballNode = createSceneNode();
    light1Node = createSceneNode();

    floor1Node = createSceneNode();
    floor2Node = createSceneNode();
    floor3Node = createSceneNode();
    floor4Node = createSceneNode();

    light1Node->nodeID = 1;
    light1Node->nodeType = SPOT_LIGHT;
    light1Node->position = light1_position;


    light1Node->normal = light1_pointed_angle;

    Mesh floorMesh = generateFloor(floor_span);
    Mesh towerMesh = generateTower(tower_dimensions);
    Mesh ballMesh = generateSphere(ball_radius, 1024, 1024);

    Mesh cube1 = cube(glm::vec3(5.0f));
    Mesh cube2 = cube(glm::vec3(10.0f));
    Mesh cube3 = cube(glm::vec3(15.0f));
    Mesh cube4 = cube(glm::vec3(20.0f));

    unsigned int floorVAO = generateBuffer(floorMesh, false);
    unsigned int towerVAO = generateBuffer(towerMesh, true);
    unsigned int ballVAO = generateBuffer(ballMesh, false);

    unsigned int testCube1VAO = generateBuffer(cube1, false);
    unsigned int testCube2VAO = generateBuffer(cube2, false);
    unsigned int testCube3VAO = generateBuffer(cube3, false);
    unsigned int testCube4VAO = generateBuffer(cube4, false);

     floorNode->vertexArrayObjectID = floorVAO;
    floorNode->VAOIndexCount = floorMesh.indices.size();
    floorNode->position = floor_position; 

    tower1Node->vertexArrayObjectID = towerVAO;
    tower1Node->VAOIndexCount = towerMesh.indices.size();
    tower1Node->position = tower_position;
    tower1Node->nodeID = 12;

    floor1Node->vertexArrayObjectID = testCube1VAO;
    floor1Node->VAOIndexCount = cube1.indices.size();
    floor1Node->position = cube1_position;
    floor2Node->vertexArrayObjectID = testCube2VAO;
    floor2Node->VAOIndexCount = cube2.indices.size();
    floor2Node->position = cube2_position;
    floor3Node->vertexArrayObjectID = testCube3VAO;
    floor3Node->VAOIndexCount = cube3.indices.size();
    floor3Node->position = cube3_position;
    floor4Node->vertexArrayObjectID = testCube4VAO;
    floor4Node->VAOIndexCount = cube4.indices.size();
    floor4Node->position = cube4_position;

     //construct normal mapped
    PNGImage brickImage = loadPNGFile("../res/textures/Brick03_col.png");
    if(brickImage.width == 0) {
        return ;
    }
    unsigned int brickTextureID = getTexture(brickImage);

    tower1Node->textureID = brickTextureID;
    
    PNGImage brickNormal = loadPNGFile("../res/textures/Brick03_nrm.png");
    if(brickNormal.width == 0) {
        return ;
    }
    unsigned int brickNormalID = getTexture(brickNormal);
    
    tower1Node->normalID = brickNormalID;
    
    tower1Node->nodeType = NORMAL_MAPPED;
    
    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = ballMesh.indices.size();
    ballNode->position = ball_position;
    ballNode->isLightSource = true;

    rootNode->children.push_back(floorNode);
    rootNode->children.push_back(floor1Node);
    rootNode->children.push_back(floor2Node);
    rootNode->children.push_back(floor3Node);
    rootNode->children.push_back(floor4Node);

    rootNode->children.push_back(tower1Node);
    
    tower1Node->children.push_back(light1Node);
    tower1Node->children.push_back(ballNode);

    printf("Node setup complete\n");
    

    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();
    float cameraSpeed = timeDelta * 60;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, upAlignment)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, upAlignment)) * cameraSpeed;
    
    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);



    glm::mat4 view = 
                    glm::lookAt(
                        cameraPosition,
                        cameraPosition + cameraFront, 
                        upAlignment
                    );

    VP = projection * view;

    pi_value = pi_value >= 2.00 ? 0.0 : pi_value + 0.25*timeDelta;

    updateNodeTransformations(rootNode, glm::mat4(1.0f));

}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0.0f,1.0f,0.0f))
            * glm::rotate(node->rotation.x, glm::vec3(1.0f,0.0f,0.0f))
            * glm::rotate(node->rotation.z, glm::vec3(0.0f,0.0f,1.0f))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;
    
    glm::mat4 mat = transformationMatrix;
    //printf("Node id: %d\n", node->nodeID);
    //std::cout<<node->nodeID<<std::endl;
    //std::cout<<glm::to_string(mat)<<std::endl;
    
    GLint lightLocation;
    GLint normalLocation;
    glm::vec3 light_pos;
    char lightBuffer[128];
    char normalBuffer[128];
    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT: 
            light_pos= glm::vec3(node->currentTransformationMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) );
            sprintf(lightBuffer, "lightSources[%d].position", node->nodeID - 1);
            lightLocation = worldShader->getUniformFromName(lightBuffer);
            glUniform3fv(lightLocation, 1, glm::value_ptr(light_pos));
            break;  
        case SPOT_LIGHT:
            node->normal = glm::normalize(glm::vec3(cos(pi_value*3.14), 0.0, sin(pi_value*3.14)));
            light_pos= glm::vec3(node->currentTransformationMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) );
            sprintf(lightBuffer, "lightSources[%d].position", node->nodeID - 1);
            lightLocation = worldShader->getUniformFromName(lightBuffer);
            glUniform3fv(lightLocation, 1, glm::value_ptr(light_pos));

            //printf("light position id: %d %g %g %g\n", node->nodeID, light_pos.x, light_pos.y, light_pos.z);
            sprintf(normalBuffer, "lightSources[%d].pointed_normal", node->nodeID - 1);
            normalLocation = worldShader->getUniformFromName(normalBuffer);
            glUniform3fv(normalLocation, 1, glm::value_ptr(node->normal));
        default: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode* node) {
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
    glm::mat3 mat = glm::mat3(glm::transpose(glm::inverse(node->currentTransformationMatrix)));
    glUniformMatrix3fv(8, 1, GL_FALSE, glm::value_ptr(mat));
    
    glUniform1i(20, node->isLightSource ? 1 : 0);

    //std::cout<<glm::to_string(mat)<<std::endl;

    glUniform1i(11, 0);
    glUniform1i(23, 0);

    switch(node->nodeType) {
        case GEOMETRY:  
            if(node->nodeID == 12) {
                glUniform1i(23, 1);
            }
            if(node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case NORMAL_MAPPED:
            //printf("Normal Mapped baby. TextureID: %d, normalID: %d\n", node->textureID, node->normalID);
            glUniform1i(11, 1);
            glBindVertexArray(node->vertexArrayObjectID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, node->textureID);
            
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, node->normalID);

            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            break;
        case POINT_LIGHT:
        case SPOT_LIGHT:
        default: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderShadows(SceneNode* node) {
    glBindVertexArray(node->vertexArrayObjectID);
    glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void setUpShadows() {
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shadowMap = depthMap;
}

void sampleShadows(GLFWwindow* window) {
    shadowShader->activate();
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    //printf("Sampling shades using shadow shader\n");
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------
    glm::mat4 projection, lightView;
    projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::vec3 light_position = light1Node->currentTransformationMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lightView = glm::lookAt(light_position, light_position + light1Node->normal, glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = projection * lightView;
    // render scene from light's point of view
    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderNode(rootNode);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void renderScene(GLFWwindow* window) {
    worldShader->activate();
    //printf("Rendering scene using world shader\n");
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(VP));
    glUniformMatrix4fv(12, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glUniform3fv(21, 1, glm::value_ptr(cameraPosition));

    renderNode(rootNode);
}


void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    sampleShadows(window);
    renderScene(window);
}

void handleInput(GLFWwindow* window) {
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
