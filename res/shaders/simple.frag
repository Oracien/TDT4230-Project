#version 430 core

struct LightSource {
    vec3 position;
    vec3 colour;
};

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec4 modelPosition;
in layout(location = 3) vec3 colour_in;
in layout(location = 4) mat3 tangentMatrix;
in layout(location = 7) vec3 tangent;
in layout(location = 8) vec3 biTangent;

layout(binding = 1) uniform sampler2D textureSampler;
layout(binding = 2) uniform sampler2D normalSampler;

uniform layout(location = 9) vec3 cameraPos;
uniform layout(location = 10) vec3 colourBuffer;

uniform LightSource lightSources[3];

out vec4 color;

vec3 normal_light_vector;
vec3 reflected_normal;
vec3 normal_view_vector;
vec3 new_normal;

float attenuation;
float dist;
vec3 position;
vec3 colour;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

void main()
{
    new_normal = normalize(normal);
    vec3 light_vector;

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    float atten1 = 0.005;
    float atten2 = 0.003;
    float atten3 = 0.001;

    float ambientStrength = 0.1;
    float diffuseStrength = 1.0;

    normal_view_vector = normalize(cameraPos - modelPosition.xyz);
    
    for (int i = 0; i < 3; i++) {
        position = lightSources[i].position;
        colour = colour_in;

        dist = distance(position, modelPosition.xyz);
        attenuation = 1 / (atten1 + dist * atten2 + dist * dist * atten3); 
        light_vector = (position - modelPosition.xyz);

        normal_light_vector = normalize(position - modelPosition.xyz);
        diffuse += colour * diffuseStrength * attenuation * max(dot(new_normal, normal_light_vector), 0);

        reflected_normal = reflect(-normal_light_vector, new_normal);
        specular += colour * attenuation * pow(max(dot(normal_view_vector, reflected_normal), 0.0), 32);
    }
    float dither_result = dither(textureCoordinates) / 1;
    
    vec3 lighting = vec3(ambientStrength + specular + diffuse + dither_result);

    color = vec4(lighting, 1.0);
    //color = vec4(colour_in, 1.0);
    //color = vec4(new_normal, 1.0);
}