#version 430 core
in layout(location = 0) vec3 position;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 0) mat4 ModelMatrix;
uniform layout(location = 1) mat4 OrthogProjectionMatrix;

out layout(location = 0) vec2 textureCoordinates_out;

void main()  
{
    gl_Position = (OrthogProjectionMatrix * vec4(position, 1.0f)) + vec4(1.0, 1.0, 0.0, 0.0);
    textureCoordinates_out = textureCoordinates_in;
}