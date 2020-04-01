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
#include "shadows.hpp"

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

const glm::vec2 floor_span = glm::vec2(400.0f, 400.0f);
const glm::vec3 floor_position = glm::vec3(-200.0f, -50.0f, 200.0f);

const glm::vec3 tower_dimensions = glm::vec3(5.0f, 45.0f, 5.0f);
const glm::vec3 tower_position = glm::vec3(-25.0f, -80.0f, -50.0f);

const glm::vec3 ball_position = glm::vec3(0.0f, 47.5f, 0.0f);
const float ball_radius = 1.0f;

const glm::vec3 light1_position = glm::vec3(-25.0f, -32.5f, -50.0f);
const glm::vec3 light1_pointed_angle = glm::vec3(1.0f, 0.0f, 0.0f);

double pi_value = 0.0;

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
GLuint shadowMap;
glm::mat4 depthProjectionMatrix; 
glm::mat4 depthViewMatrix;
glm::mat4 depthModelMatrix = glm::mat4(1.0);
glm::mat4 depthMVP;
glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);
glm::mat4 depthBiasMVP;

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
    //glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
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
    
    printf("Constructing SceneNodes\n");
    // Construct scene
    rootNode = createSceneNode();
    floorNode = createSceneNode();
    tower1Node = createSceneNode();
    ballNode = createSceneNode();
    light1Node = createSceneNode();

    light1Node->nodeID = 1;
    light1Node->nodeType = SPOT_LIGHT;
    light1Node->position = light1_position;


    light1Node->normal = light1_pointed_angle;

    Mesh floorMesh = generateFloor(floor_span);
    Mesh towerMesh = generateTower(tower_dimensions);
    Mesh ballMesh = generateSphere(ball_radius, 1024, 1024);

    unsigned int floorVAO = generateBuffer(floorMesh);
    unsigned int towerVAO = generateBuffer(towerMesh);
    unsigned int ballVAO = generateBuffer(ballMesh);

    floorNode->vertexArrayObjectID = floorVAO;
    floorNode->VAOIndexCount = floorMesh.indices.size();
    floorNode->position = floor_position;

    tower1Node->vertexArrayObjectID = towerVAO;
    tower1Node->VAOIndexCount = towerMesh.indices.size();
    tower1Node->position = tower_position;
    tower1Node->nodeID = 12;

    /* //construct normal mapped
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
    */
    shadowMap = setupShadows();
    
    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = ballMesh.indices.size();
    ballNode->position = ball_position;
    ballNode->isLightSource = true;

    rootNode->children.push_back(floorNode);
    rootNode->children.push_back(tower1Node);
    rootNode->children.push_back(light1Node);

    //tower1Node->children.push_back(ballNode);

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

    pi_value = pi_value >= 2.00 ? 0.0 : pi_value + 0.001;

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
            node->normal = glm::normalize(glm::vec3(cos(pi_value*3.14), -0.2, sin(pi_value*3.14)));
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

    switch(node->nodeType) {
        case GEOMETRY:  
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

void sampleShadows() {
    //printf("Sampling shades using shadow shader\n");
    shadowShader->activate();
    
    depthProjectionMatrix = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    depthViewMatrix = glm::lookAt(light1Node->position, light1Node->normal, glm::vec3(0, 1, 0));
    depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
    depthBiasMVP = biasMatrix*depthMVP;
    

    glUniformMatrix4fv(0, 1, GL_FALSE, &depthMVP[0][0]);
    renderShadows(rootNode);

}

void renderScene() {
    //printf("Rendering scene using world shader\n");
    worldShader->activate();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadowMap);

    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(VP));
    glUniformMatrix4fv(12, 1, GL_FALSE, glm::value_ptr(depthBiasMVP));
    glUniform3fv(21, 1, glm::value_ptr(cameraPosition));

    renderNode(rootNode);
}


void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    sampleShadows();
    renderScene();
}

void handleInput(GLFWwindow* window) {
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
