#include <iostream>
#include "shapes.h"
#include "SimplexNoise.h"
#define M_PI 3.14159265359f

const int floor_horizontal_section = 2;
const int floor_vertical_section = 2;
const glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);

enum Direction {
    HORIZONTAL, VERTICAL, DEPTH
};

const std::vector<glm::vec3> colors = {
    glm::vec3(0.1, 0.2, 0.1), 
    glm::vec3(0.3, 0.2, 0.1), 
    glm::vec3(0.5, 0.2, 0.1), 
    glm::vec3(0.6, 0.4, 0.1), 
    glm::vec3(0.3, 0.5, 0.7), 
    glm::vec3(0.3, 0.5, 0.9), 
    glm::vec3(0.5, 0.7, 1.0), 
    glm::vec3(0.7, 0.9, 1.0)
};

Mesh cube(glm::vec3 scale, glm::vec2 textureScale, bool tilingTextures, bool inverted, glm::vec3 textureScale3d) {
    glm::vec3 points[8];
    int indices[36];

    for (int y = 0; y <= 1; y++)
    for (int z = 0; z <= 1; z++)
    for (int x = 0; x <= 1; x++) {
        points[x+y*4+z*2] = glm::vec3(x*2-1, y*2-1, z*2-1) * 0.5f * scale;
    }

    int faces[6][4] = {
        {2,3,0,1}, // Bottom 
        {4,5,6,7}, // Top 
        {7,5,3,1}, // Right 
        {4,6,0,2}, // Left 
        {5,4,1,0}, // Back 
        {6,7,2,3}, // Front 
    };

    scale = scale * textureScale3d;
    glm::vec2 faceScale[6] = {
        {-scale.x,-scale.z}, // Bottom
        {-scale.x,-scale.z}, // Top
        { scale.z, scale.y}, // Right
        { scale.z, scale.y}, // Left
        { scale.x, scale.y}, // Back
        { scale.x, scale.y}, // Front
    }; 

    glm::vec3 normals[6] = {
        { 0,-1, 0}, // Bottom 
        { 0, 1, 0}, // Top 
        { 1, 0, 0}, // Right 
        {-1, 0, 0}, // Left 
        { 0, 0,-1}, // Back 
        { 0, 0, 1}, // Front 
    };

    glm::vec2 UVs[4] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1},
    };

    Mesh m;
    for (int face = 0; face < 6; face++) {
        int offset = face * 6;
        indices[offset + 0] = faces[face][0];
        indices[offset + 3] = faces[face][0];

        if (!inverted) {
            indices[offset + 1] = faces[face][3];
            indices[offset + 2] = faces[face][1];
            indices[offset + 4] = faces[face][2];
            indices[offset + 5] = faces[face][3];
        } else {
            indices[offset + 1] = faces[face][1];
            indices[offset + 2] = faces[face][3];
            indices[offset + 4] = faces[face][3];
            indices[offset + 5] = faces[face][2];
        }

        for (int i = 0; i < 6; i++) {
            m.vertices.push_back(points[indices[offset + i]]);
            m.indices.push_back(offset + i);
            m.normals.push_back(normals[face] * (inverted ? -1.f : 1.f));
        }

        glm::vec2 textureScaleFactor = tilingTextures ? (faceScale[face] / textureScale) : glm::vec2(1);

        if (!inverted) {
            for (int i : {1,2,3,1,0,2}) {
                m.textureCoordinates.push_back(UVs[i] * textureScaleFactor);
            }
        } else {
            for (int i : {3,1,0,3,0,2}) {
                m.textureCoordinates.push_back(UVs[i] * textureScaleFactor);
            }
        }
    }

    return m;
}

Mesh generateRectangle(glm::vec3 starting, glm::vec2 dimensions, Direction direction, int &vertex_count) {
    Mesh m;

    switch (direction)
    {
    case VERTICAL:
        m.vertices.push_back(glm::vec3(starting.x               , starting.y                , starting.z));
        m.vertices.push_back(glm::vec3(starting.x               , starting.y + dimensions.y , starting.z));
        m.vertices.push_back(glm::vec3(starting.x + dimensions.x, starting.y                , starting.z));
        m.vertices.push_back(glm::vec3(starting.x + dimensions.x, starting.y + dimensions.y , starting.z));
        break;

    case HORIZONTAL:
        m.vertices.push_back(glm::vec3(starting.x               , starting.y, starting.z));
        m.vertices.push_back(glm::vec3(starting.x               , starting.y, starting.z-dimensions.y));
        m.vertices.push_back(glm::vec3(starting.x + dimensions.x, starting.y, starting.z));
        m.vertices.push_back(glm::vec3(starting.x + dimensions.x, starting.y, starting.z-dimensions.y));
        break;

    case DEPTH:
        m.vertices.push_back(glm::vec3(starting.x, starting.y               , starting.z));
        m.vertices.push_back(glm::vec3(starting.x, starting.y + dimensions.x, starting.z));
        m.vertices.push_back(glm::vec3(starting.x, starting.y               , starting.z-dimensions.y));
        m.vertices.push_back(glm::vec3(starting.x, starting.y + dimensions.x, starting.z-dimensions.y));
        break;
    
    default:
        break;
    }
    m.indices.push_back(vertex_count + 0);
    m.indices.push_back(vertex_count + 3);
    m.indices.push_back(vertex_count + 1);
    
    m.indices.push_back(vertex_count + 0);
    m.indices.push_back(vertex_count + 2);
    m.indices.push_back(vertex_count + 3);

    m.normals.push_back(glm::normalize(glm::cross(
        m.vertices[3] - m.vertices[0],
        m.vertices[1] - m.vertices[0]
    )));


    m.normals.push_back(glm::normalize(glm::cross(
        m.vertices[2] - m.vertices[0],
        m.vertices[3] - m.vertices[0]
    )));

    vertex_count += 4;

    m.textureCoordinates.push_back(glm::vec2(0, 0));
    m.textureCoordinates.push_back(glm::vec2(0, 1));
    m.textureCoordinates.push_back(glm::vec2(1, 0));
    m.textureCoordinates.push_back(glm::vec2(1, 1));

    for (unsigned int i = 0; i < m.vertices.size(); i++) {
        m.colours.push_back(glm::vec3(0.6f, 0.6f, 0.6f));
    }

    return m;
}

void extendVector(std::vector<glm::vec3> &list1, std::vector<glm::vec3> list2) {
    list1.reserve(list1.size() + std::distance(list2.begin(), list2.end()));
    list1.insert(list1.end(), list2.begin(), list2.end());
}

void extendVector(std::vector<glm::vec2> &list1, std::vector<glm::vec2> list2) {
    list1.reserve(list1.size() + std::distance(list2.begin(), list2.end()));
    list1.insert(list1.end(), list2.begin(), list2.end());
}

void extendVector(std::vector<unsigned int> &list1, std::vector<unsigned int> list2) {
    list1.reserve(list1.size() + std::distance(list2.begin(), list2.end()));
    list1.insert(list1.end(), list2.begin(), list2.end());
}

void extendIndices(std::vector<unsigned int> &indices1, std::vector<unsigned int> indices2, unsigned int vertext_count) {
    for (unsigned int i = 0; i < indices2.size(); i++) {
        indices1.push_back(indices2[i] + vertext_count);
    }
}

Mesh generateCuboidOld(glm::vec3 position, glm::vec3 dimensions) {
    std::vector<Mesh> meshes;

    Mesh m;
    int vertex_count = 0;

    meshes.push_back(generateRectangle(position, glm::vec2(dimensions.x, dimensions.z), HORIZONTAL, vertex_count));
    meshes.push_back(generateRectangle(position, glm::vec2(dimensions.x, dimensions.y), VERTICAL, vertex_count));
    meshes.push_back(generateRectangle(position, glm::vec2(dimensions.y, dimensions.z), DEPTH, vertex_count));
    meshes.push_back(generateRectangle(position + glm::vec3(dimensions.x, 0, 0), glm::vec2(dimensions.y, dimensions.z), DEPTH, vertex_count));
    meshes.push_back(generateRectangle(position + glm::vec3(0, 0, -dimensions.z), glm::vec2(dimensions.x, dimensions.y), VERTICAL, vertex_count));
    meshes.push_back(generateRectangle(position + glm::vec3(0, dimensions.y, 0), glm::vec2(dimensions.x, dimensions.z), HORIZONTAL, vertex_count));

    //printf("lengths before: %d %d %d %d, %d\n", m.vertices.size(), m.indices.size(), m.textureCoordinates.size(), m.colours.size(), m.normals.size());
    for (unsigned int i = 0; i < meshes.size(); i++) {
        extendVector(m.vertices, meshes[i].vertices);
        extendVector(m.indices, meshes[i].indices);
        extendVector(m.textureCoordinates, meshes[i].textureCoordinates);
        extendVector(m.colours, meshes[i].colours);
        extendVector(m.normals, meshes[i].normals);
    }
    //printf("lengths after: %d %d %d %d, %d\n", m.vertices.size(), m.indices.size(), m.textureCoordinates.size(), m.colours.size(), m.normals.size());
    return m;
}

void extendMesh(Mesh &mesh1, Mesh mesh2) {
    extendVector(mesh1.vertices, mesh2.vertices);
    extendIndices(mesh1.indices, mesh2.indices, mesh1.vertices.size());
    extendVector(mesh1.textureCoordinates, mesh2.textureCoordinates);
    extendVector(mesh1.colours, mesh2.colours);
    extendVector(mesh1.normals, mesh2.normals);
}

Mesh generateCuboid(glm::vec3 position, glm::vec3 dimensions) {
    Mesh m;


    for (int y = 0; y < 2; y++) {
        for (int z = 0; z < 2; z++) {
            for (int x = 0; x < 2; x++) {
                m.vertices.push_back(position + glm::vec3(dimensions.x * x, dimensions.y * y, -dimensions.z * z));
            }
        }
    }
    /*
    Bottom:
    2 3
    0 1

    Top:
    6 7
    4 5
    */

    //Bottom surface
    m.indices.push_back(0);
    m.indices.push_back(3);
    m.indices.push_back(1);
    m.indices.push_back(0);
    m.indices.push_back(2);
    m.indices.push_back(3);

    //Top surface
    m.indices.push_back(4);
    m.indices.push_back(7);
    m.indices.push_back(6);
    m.indices.push_back(4);
    m.indices.push_back(5);
    m.indices.push_back(7);
    
    //Front surface
    m.indices.push_back(0);
    m.indices.push_back(5);
    m.indices.push_back(4);
    m.indices.push_back(0);
    m.indices.push_back(1);
    m.indices.push_back(5);
    
    //Left surface
    m.indices.push_back(2);
    m.indices.push_back(4);
    m.indices.push_back(6);
    m.indices.push_back(2);
    m.indices.push_back(0);
    m.indices.push_back(4);
    
    //Right surface
    m.indices.push_back(1);
    m.indices.push_back(7);
    m.indices.push_back(5);
    m.indices.push_back(1);
    m.indices.push_back(3);
    m.indices.push_back(7);
    
    //Back surface
    m.indices.push_back(3);
    m.indices.push_back(6);
    m.indices.push_back(7);
    m.indices.push_back(3);
    m.indices.push_back(2);
    m.indices.push_back(6);

    //Constant order, bottom left, top right, top left, bottom left, bottom right, top right.
    for (int i = 0; i < 6; i++) {
        m.textureCoordinates.push_back({0, 0});
        m.textureCoordinates.push_back({1, 1});
        m.textureCoordinates.push_back({0, 1});
        m.textureCoordinates.push_back({0, 0});
        m.textureCoordinates.push_back({1, 0});
        m.textureCoordinates.push_back({1, 1});

        m.colours.push_back(glm::vec3(1.0, 0.0, 0.0));
        m.colours.push_back(glm::vec3(0.0, 1.0, 0.0));
        m.colours.push_back(glm::vec3(0.0, 1.0, 0.0));
        m.colours.push_back(glm::vec3(1.0, 0.0, 0.0));
        m.colours.push_back(glm::vec3(1.0, 0.0, 0.0));
        m.colours.push_back(glm::vec3(0.0, 1.0, 0.0));
    }

    //Ordering: Bottom, top, front, left, right, back
    for(int i = 0; i < 6; i++) {
        m.normals.push_back(glm::vec3(0, -1, 0));
    }
    for(int i = 0; i < 6; i++) {
        m.normals.push_back(glm::vec3(0, 1, 0));
    }      
    for(int i = 0; i < 6; i++) {
        m.normals.push_back(glm::vec3(0, 0, 1));
    }      
    for(int i = 0; i < 6; i++) {
        m.normals.push_back(glm::vec3(-1, 0, 0));
    }      
    for(int i = 0; i < 6; i++) {
        m.normals.push_back(glm::vec3(1, 0, 0));
    }      
    for(int i = 0; i < 6; i++) {
        m.normals.push_back(glm::vec3(0, 0, -1));
    }

    printf("Sizes: vertices: %ld, indicies: %ld, text: %ld, colours: %ld, normals: %ld\n", m.vertices.size(), m.indices.size(), m.textureCoordinates.size(), m.colours.size(), m.normals.size());

    return m;
}


Mesh generateTower(glm::vec3 dimensions) {
    Mesh tower = Mesh();

    glm::vec3 centering = glm::vec3(- dimensions.x /2, 0, dimensions.z /2);
    Mesh towerBase =    generateCuboid(centering + origin, dimensions);
    Mesh pillar1 =      generateCuboid(centering + glm::vec3(0,                  dimensions.y,      0),                 glm::vec3(1, 5, 1));
    Mesh pillar2 =      generateCuboid(centering + glm::vec3(dimensions.x - 1,   dimensions.y,      0),                 glm::vec3(1, 5, 1));
    Mesh pillar3 =      generateCuboid(centering + glm::vec3(0,                  dimensions.y,      -dimensions.z + 1), glm::vec3(1, 5, 1));
    Mesh pillar4 =      generateCuboid(centering + glm::vec3(dimensions.x - 1,   dimensions.y,      -dimensions.z + 1), glm::vec3(1, 5, 1));
    Mesh top =          generateCuboid(centering + glm::vec3(0,                  dimensions.y + 5,  0),                 glm::vec3(dimensions.x, 2, dimensions.z));


    printf("lengths before: %ld %ld %ld %ld %ld\n", tower.vertices.size(), tower.indices.size(), tower.textureCoordinates.size(), tower.colours.size(), tower.normals.size());
    extendMesh(tower, towerBase);
    extendMesh(tower, pillar1);
    extendMesh(tower, pillar2);
    extendMesh(tower, pillar3);
    extendMesh(tower, pillar4);
    extendMesh(tower, top);
    printf("lengths after: %ld %ld %ld %ld %ld\n", tower.vertices.size(), tower.indices.size(), tower.textureCoordinates.size(), tower.colours.size(), tower.normals.size());

    return tower;
}

glm::vec3 getColourFromNoise( float noise) {
    int possible_colour = colors.size();
    float incremenet = (0.80*2) /((float) possible_colour);
    for (int i = 0; i < possible_colour; i++) {
        if(noise < -0.82 + incremenet * (i + 1)) {
            return colors[i];
        }
    }
    return colors[0];
}

Mesh generateFloor( glm::vec2 scale) {
    Mesh m;

    float height_scale = 20.0f;

    SimplexNoise simplex = SimplexNoise();

    for (int a = 0; a >= -scale.y; a -= floor_vertical_section) {
        for (int b = 0; b <= scale.x; b+= floor_horizontal_section) {
            float noise = simplex.fractal(8, a*0.01, b*0.01);
            m.vertices.push_back(glm::vec3(b, noise*height_scale, a));
            glm::vec3 color = getColourFromNoise(noise);
            m.colours.push_back(color);
        }
    }
    
    int alength = (int) scale.y / floor_vertical_section;
    int blength = (int) scale.x / floor_horizontal_section;
    int next_row = blength + 1;
    int next_column = 1;
    for (int a = 0; a < alength; a++) {
        int offset = a * next_row;
        for (int b = 0; b < blength; b++) {
            int bottom_left =   offset + b;
            int top_right =     offset + next_row + b + next_column;
            int top_left =      offset + next_row + b;
            int bottom_right =  offset + b + next_column;
            //Bottom Left
            m.indices.push_back(bottom_left);
            //Top Right
            m.indices.push_back(top_right);
            //Top Left
            m.indices.push_back(top_left);
            //Bottom Left
            m.indices.push_back(bottom_left);
            //Bottom Right
            m.indices.push_back(bottom_right);
            //Top Right
            m.indices.push_back(top_right);

            
            m.textureCoordinates.push_back({0, 0});
            m.textureCoordinates.push_back({1, 1});
            m.textureCoordinates.push_back({1, 0});
            m.textureCoordinates.push_back({0, 0});
            m.textureCoordinates.push_back({0, 1});
            m.textureCoordinates.push_back({1, 1});

            m.normals.push_back(glm::normalize(glm::cross
            ((m.vertices[top_right] - m.vertices[bottom_left]),
            (m.vertices[top_left] - m.vertices[bottom_left]))));

            m.normals.push_back(glm::normalize(glm::cross
            ((m.vertices[top_left] - m.vertices[top_right]),
            (m.vertices[bottom_left] - m.vertices[top_right]))));

            m.normals.push_back(glm::normalize(glm::cross
            ((m.vertices[bottom_left] - m.vertices[top_left]),
            (m.vertices[top_right] - m.vertices[top_left]))));

            m.normals.push_back(glm::normalize(glm::cross
            ((m.vertices[top_right] - m.vertices[bottom_left]),
            (m.vertices[bottom_right] - m.vertices[bottom_left]))));

            m.normals.push_back(glm::normalize(glm::cross
            ((m.vertices[bottom_left] - m.vertices[bottom_right]),
            (m.vertices[top_right] - m.vertices[bottom_right]))));

            m.normals.push_back(glm::normalize(glm::cross
            ((m.vertices[bottom_left] - m.vertices[top_right]),
            (m.vertices[bottom_right] - m.vertices[top_right]))));
        }
    }
    
    return m;
}

Mesh generateSphere(float sphereRadius, int slices, int layers) {
    const unsigned int triangleCount = slices * layers * 2;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    std::vector<glm::vec2> uvs;

    vertices.reserve(3 * triangleCount);
    normals.reserve(3 * triangleCount);
    indices.reserve(3 * triangleCount);

    // Slices require us to define a full revolution worth of triangles.
    // Layers only requires angle varying between the bottom and the top (a layer only covers half a circle worth of angles)
    const float degreesPerLayer = 180.0 / (float) layers;
    const float degreesPerSlice = 360.0 / (float) slices;

    unsigned int i = 0;

    // Constructing the sphere one layer at a time
    for (int layer = 0; layer < layers; layer++) {
        int nextLayer = layer + 1;

        // Angles between the vector pointing to any point on a particular layer and the negative z-axis
        float currentAngleZDegrees = degreesPerLayer * layer;
        float nextAngleZDegrees = degreesPerLayer * nextLayer;

        // All coordinates within a single layer share z-coordinates.
        // So we can calculate those of the current and subsequent layer here.
        float currentZ = -cos(glm::radians(currentAngleZDegrees));
        float nextZ = -cos(glm::radians(nextAngleZDegrees));

        // The row of vertices forms a circle around the vertical diagonal (z-axis) of the sphere.
        // These radii are also constant for an entire layer, so we can precalculate them.
        float radius = sin(glm::radians(currentAngleZDegrees));
        float nextRadius = sin(glm::radians(nextAngleZDegrees));

        // Now we can move on to constructing individual slices within a layer
        for (int slice = 0; slice < slices; slice++) {

            // The direction of the start and the end of the slice in the xy-plane
            float currentSliceAngleDegrees = slice * degreesPerSlice;
            float nextSliceAngleDegrees = (slice + 1) * degreesPerSlice;

            // Determining the direction vector for both the start and end of the slice
            float currentDirectionX = cos(glm::radians(currentSliceAngleDegrees));
            float currentDirectionY = sin(glm::radians(currentSliceAngleDegrees));

            float nextDirectionX = cos(glm::radians(nextSliceAngleDegrees));
            float nextDirectionY = sin(glm::radians(nextSliceAngleDegrees));

            vertices.emplace_back(sphereRadius * radius * currentDirectionX,
                                  sphereRadius * radius * currentDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * radius * nextDirectionX,
                                  sphereRadius * radius * nextDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * nextRadius * nextDirectionX,
                                  sphereRadius * nextRadius * nextDirectionY,
                                  sphereRadius * nextZ);
            vertices.emplace_back(sphereRadius * radius * currentDirectionX,
                                  sphereRadius * radius * currentDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * nextRadius * nextDirectionX,
                                  sphereRadius * nextRadius * nextDirectionY,
                                  sphereRadius * nextZ);
            vertices.emplace_back(sphereRadius * nextRadius * currentDirectionX,
                                  sphereRadius * nextRadius * currentDirectionY,
                                  sphereRadius * nextZ);

            normals.emplace_back(radius * currentDirectionX,
                                 radius * currentDirectionY,
                                 currentZ);
            normals.emplace_back(radius * nextDirectionX,
                                 radius * nextDirectionY,
                                 currentZ);
            normals.emplace_back(nextRadius * nextDirectionX,
                                 nextRadius * nextDirectionY,
                                 nextZ);
            normals.emplace_back(radius * currentDirectionX,
                                 radius * currentDirectionY,
                                 currentZ);
            normals.emplace_back(nextRadius * nextDirectionX,
                                 nextRadius * nextDirectionY,
                                 nextZ);
            normals.emplace_back(nextRadius * currentDirectionX,
                                 nextRadius * currentDirectionY,
                                 nextZ);

            indices.emplace_back(i + 0);
            indices.emplace_back(i + 1);
            indices.emplace_back(i + 2);
            indices.emplace_back(i + 3);
            indices.emplace_back(i + 4);
            indices.emplace_back(i + 5);

            for (int j = 0; j < 6; j++) {
                glm::vec3 vertex = vertices.at(i+j);
                uvs.emplace_back(
                    0.5 + (glm::atan(vertex.z, vertex.y)/(2.0*M_PI)),
                    0.5 - (glm::asin(vertex.y)/M_PI)
                );
            }

            i += 6;
        }
    }

    Mesh mesh;
    mesh.vertices = vertices;
    mesh.normals = normals;
    mesh.indices = indices;
    mesh.textureCoordinates = uvs;
    return mesh;
}
