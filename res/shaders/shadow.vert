#version 430 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

// Values that stay constant for the whole mesh.
uniform layout(location = 0) mat4 depthMVP;

void main(){
    gl_Position =  depthMVP * vec4(vertexPosition_modelspace,1);
}