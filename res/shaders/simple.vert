#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent;
in layout(location = 4) vec3 biTangent;
in layout(location = 5) vec3 colour_in;

uniform layout(location = 0) mat4 ModelMatrix;
uniform layout(location = 4) mat4 ViewProjectionMatrix;
uniform layout(location = 8) mat3 NormalMatrix;
uniform layout(location = 11) int NormalMapToggle;
uniform layout(location = 12) mat4 lightSpaceMatrix;


out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec4 model_pos_out;
out layout(location = 3) vec3 colour_out;
flat out layout(location = 4) int NormalMapToggleOut;
out layout(location = 5) mat3 tangentMatrix;
out layout(location = 8) vec3 outTangent;
out layout(location = 9) vec3 outBiTangent;
out layout(location = 10) vec4 shadowCoord;

void main()
{
    normal_out = normalize(NormalMatrix * normal_in);
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = ViewProjectionMatrix * ModelMatrix * vec4(position, 1.0f);
    model_pos_out = ModelMatrix * vec4(position, 1.0f);
    colour_out = colour_in;
    NormalMapToggleOut = NormalMapToggle;
    tangentMatrix = (mat3(normalize(tangent), normalize(biTangent), normalize(normal_in)));
    outTangent = tangent;
    outBiTangent = biTangent;
    shadowCoord = lightSpaceMatrix * ModelMatrix * vec4(position, 1.0f);
}
