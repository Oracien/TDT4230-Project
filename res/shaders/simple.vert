#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 5) vec3 colour_in;

uniform layout(location = 0) mat4 ModelMatrix;
uniform layout(location = 1) mat4 ViewProjectionMatrix;
uniform layout(location = 2) mat3 NormalMatrix;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec4 model_pos_out;
out layout(location = 3) vec3 colour_out;

void main()
{
    normal_out = normalize(NormalMatrix * normal_in);
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = ViewProjectionMatrix * ModelMatrix * vec4(position, 1.0f);
    model_pos_out = ModelMatrix * vec4(position, 1.0f);
    colour_out = colour_in;
}