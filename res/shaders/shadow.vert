#version 430 core
layout (location = 0) in vec3 aPos;

uniform layout(location = 0) mat4 model;
uniform layout(location = 4) mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}