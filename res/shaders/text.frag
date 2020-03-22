#version 430 core

in layout(location = 0) vec2 textureCoordinates;

layout(binding = 0) uniform sampler2D textSampler;

out vec4 color;

void main() 
{
    //color = vec4(1.0, 0.0, 0.0, 1.0);
    color = texture(textSampler, textureCoordinates);
}