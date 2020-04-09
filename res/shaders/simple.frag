#version 430 core

struct LightSource {
    vec3 position;
    vec3 pointed_normal;
};

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec4 modelPosition;
in layout(location = 3) vec3 colour_in;
flat in layout(location = 4) int NormalMapToggle;
in layout(location = 5) mat3 tangentMatrix;
in layout(location = 8) vec3 tangent;
in layout(location = 9) vec3 biTangent;
in layout(location = 10) vec4 shadowCoord;

layout(binding = 1) uniform sampler2D textureSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D shadowMap;

uniform layout(location = 20) int LightOrb;
uniform layout(location = 21) vec3 cameraPos;
uniform layout(location = 22) vec3 colourBuffer;

uniform LightSource lightSources[3];

out vec4 color;

vec3 normal_light_vector;
vec3 reflected_normal;
vec3 normal_view_vector;
vec3 new_normal;

float visibility;
float attenuation;
float dist;
vec3 position;
vec3 colour;
float bias = 0.005;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    if(NormalMapToggle == 1) {
        new_normal = normalize(tangentMatrix * ((texture(normalSampler, textureCoordinates).xyz * 2.0f) - 1.0f));
    } else {
        new_normal = normalize(normal);
    }
    vec3 light_vector;

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    float atten1 = 0.005;
    float atten2 = 0.003;
    float atten3 = 0.001;

    float ambientStrength = 0.1;
    float diffuseStrength = 1.0;
    float specularStrength = 1.0;
    float specularPower = 32;

    visibility = 1.0f;

    if (textureProj( shadowMap, shadowCoord.xyw).z < (shadowCoord.z-bias) /shadowCoord.w) {
        visibility = 0.0f;
    }

    normal_view_vector = normalize(cameraPos - modelPosition.xyz);
    
    for (int i = 0; i < 1; i++) {
        position = lightSources[i].position;
        colour = colour_in;

        light_vector = (position - modelPosition.xyz);

        /* if ((dot(normalize(light_vector), normalize(lightSources[i].pointed_normal))) < (3.14/4.0)) {
            continue;
        } */
        dist = distance(position, modelPosition.xyz);
        attenuation = 1 / (atten1 + dist * atten2 + dist * dist * atten3); 

        normal_light_vector = normalize(position - modelPosition.xyz);
        diffuse += colour * diffuseStrength * attenuation * max(dot(new_normal, normal_light_vector), 0);

        reflected_normal = reflect(-normal_light_vector, new_normal);
        specular += colour * specularStrength * attenuation * pow(max(dot(normal_view_vector, reflected_normal), 0.0), specularPower);
    }
    float dither_result = dither(textureCoordinates) / 1;

    vec3 ambient =  ambientStrength * colour_in;

    float shadow = ShadowCalculation(shadowCoord);    
    
    vec3 lighting = vec3(ambient + (1.0 - shadow) * (specular + diffuse + dither_result));
    lighting.x = lighting.x > 1.0 ? 1.0 : lighting.x;
    lighting.y = lighting.y > 1.0 ? 1.0 : lighting.y;
    lighting.z = lighting.z > 1.0 ? 1.0 : lighting.z;

    if(LightOrb == 1) {
        color = vec4(1.0);
    } else if(NormalMapToggle == 1) {
        color = vec4(texture(textureSampler, textureCoordinates).xyz * lighting, 1.0f);
    } else {
        color = vec4(lighting, 1.0);
    }
}