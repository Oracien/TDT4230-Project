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

const int NO_OF_LIGHT_SOURCES = 3;

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
const glm::vec3 floor_position = glm::vec3(-200.0f, -50.0f, 0.0f);

const glm::vec3 tower_dimensions = glm::vec3(10.0f, 25.0f, 10.0f);
const glm::vec3 tower_position = glm::vec3(-25.0f, -50.0f, -50.0f);

const glm::vec3 ball_position = glm::vec3(0.0f, 27.5f, 0.0f);
const float ball_radius = 1.5f;

const glm::vec3 light1_position = glm::vec3(0.0f, 27.5f, 0.0f);
const glm::vec3 light2_position = glm::vec3(0.0f, -22.0f, -150.0f);
const glm::vec3 light3_position = glm::vec3(0.0f, -22.0f, -300.0f);

glm::vec4 lightArray[NO_OF_LIGHT_SOURCES];

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* worldShader;
sf::Sound* sound;

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
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    worldShader = new Gloom::Shader();
    worldShader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    worldShader->activate();

    // Construct scene
    rootNode = createSceneNode();
    floorNode = createSceneNode();
    tower1Node = createSceneNode();
    ballNode = createSceneNode();
    light1Node = new SceneNode(1);
    light2Node = new SceneNode(2);
    light3Node = new SceneNode(3);

    light1Node->position = light1_position;
    light2Node->position = light2_position;
    light3Node->position = light3_position;

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

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = ballMesh.indices.size();
    ballNode->position = ball_position;
    ballNode->isLightSource = true;

    rootNode->children.push_back(floorNode);
    rootNode->children.push_back(tower1Node);

    tower1Node->children.push_back(light1Node);
    tower1Node->children.push_back(ballNode);
    rootNode->children.push_back(light2Node);
    rootNode->children.push_back(light3Node);


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


    glUniform3fv(21, 1, glm::value_ptr(cameraPosition));

    glm::mat4 view = 
                    glm::lookAt(
                        cameraPosition,
                        cameraPosition + cameraFront, 
                        upAlignment
                    );

    glm::mat4 VP = projection * view;


    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(VP));
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
    //std::cout<<glm::to_string(mat)<<std::endl;
    
    GLint lightLocation;
    GLint colourLocation;
    glm::vec3 light_pos;
    glm::vec3 colour;
    char lightBuffer[128];
    char colourBuffer[128];
    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT: 
            light_pos= glm::vec3(node->currentTransformationMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) );
            sprintf(lightBuffer, "lightSources[%d].position", node->nodeID - 1);
            lightLocation = worldShader->getUniformFromName(lightBuffer);
            glUniform3fv(lightLocation, 1, glm::value_ptr(light_pos));

            printf("light position id: %d %g %g %g\n", node->nodeID, light_pos.x, light_pos.y, light_pos.z);
            sprintf(colourBuffer, "lightSources[%d].colour", node->nodeID - 1);
            colourLocation = worldShader->getUniformFromName(colourBuffer);
            switch(node->nodeID) {
                case 1:
                    colour = glm::vec3(1.0f, 0.0f, 0.0f);
                    break;
                case 2:
                    colour = glm::vec3(0.0f, 1.0f, 0.0f);
                    break; 
                case 3:
                    colour = glm::vec3(0.0f, 0.0f, 1.0f);
                    break;
                default:
                    colour = glm::vec3(1.0f, 1.0f, 1.0f);
                    break;
            }
            glUniform3fv(colourLocation, 1, glm::value_ptr(colour));
            break;  
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


void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    renderNode(rootNode);
}

void handleInput(GLFWwindow* window) {
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
