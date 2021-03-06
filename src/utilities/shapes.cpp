#include <iostream>
#include "shapes.h"
#include "SimplexNoise.h"
#define M_PI 3.14159265359f

const int floor_horizontal_section = 1;
const int floor_depth_section = 1;
const glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);

enum Direction {
    HORIZONTAL, VERTICAL, DEPTH
};

const std::vector<glm::vec3> colors = {
    glm::vec3(0.24, 0.36, 0.51), //water dark blue
    glm::vec3(0.4, 0.6, 0.85), //water light blue
    glm::vec3(0.89, 0.77, 0.65), //sand
    glm::vec3(0.22, 0.41, 0.14), //grass light green
    glm::vec3(0.01, 0.26, 0.0), //grass dark green
    glm::vec3(0.14, 0.2, 0.08), //grass darker green
    glm::vec3(0.24, 0.2, 0.14), //dirt dark brown
    glm::vec3(0.42, 0.35, 0.24), //dirt brown
    glm::vec3(0.20, 0.20, 0.20), //mountain darkerer grey
    glm::vec3(0.25, 0.26, 0.26), //moutain darker grey
    glm::vec3(0.39, 0.41, 0.41), //mountain dark grey
    glm::vec3(0.47, 0.55, 0.67), //mountain blue grey
    glm::vec3(0.76, 0.8, 0.8), //mountain light grey
    glm::vec3(0.73, 0.87, 1.0), //snow light blue
    glm::vec3(0.97, 1.0, 1.0), //snow less white
    glm::vec3(1.0, 1.0, 1.0) //snow white
};

Mesh cube_with_offset(glm::vec3 scale, glm::vec3 offset, glm::vec3 textureScale) {
    Mesh qube = cube(scale, glm::vec2(1.0), true, false, textureScale);

    for (int i = 0; i < qube.vertices.size(); i++) {
        qube.vertices.at(i) = qube.vertices[i] + offset;
    }
    return qube;
}

Mesh cube(glm::vec3 scale, glm::vec2 textureScale, bool tilingTextures, bool inverted, glm::vec3 textureScale3d) {
    glm::vec3 points[8];
    int indices[36];

    for (int y = 0; y <= 1; y++)
    for (int z = 0; z <= 1; z++)
    for (int x = 0; x <= 1; x++) {
        points[x+y*4+z*2] = glm::vec3(x, y, z) * scale;
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
        { 0.0f,-1.0f, 0.0f}, // Bottom 
        { 0.0f, 1.0f, 0.0f}, // Top 
        { 1.0f, 0.0f, 0.0f}, // Right 
        {-1.0f, 0.0f, 0.0f}, // Left 
        { 0.0f, 0.0f,-1.0f}, // Back 
        { 0.0f, 0.0f, 1.0f}, // Front 
    };

    glm::vec3 colors[6] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
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
            m.colours.push_back(glm::vec3(0.3));
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

std::vector<glm::vec3> extendVector(std::vector<glm::vec3> list1, std::vector<glm::vec3> list2) {
    list1.reserve(list1.size() + std::distance(list2.begin(), list2.end()));
    list1.insert(list1.end(), list2.begin(), list2.end());
    return list1;
}

std::vector<glm::vec2> extendVector(std::vector<glm::vec2> list1, std::vector<glm::vec2> list2) {
    list1.reserve(list1.size() + std::distance(list2.begin(), list2.end()));
    list1.insert(list1.end(), list2.begin(), list2.end());
    return list1;
}

std::vector<unsigned int> extendVector(std::vector<unsigned int> list1, std::vector<unsigned int> list2) {
    list1.reserve(list1.size() + std::distance(list2.begin(), list2.end()));
    list1.insert(list1.end(), list2.begin(), list2.end());
    return list1;
}

std::vector<unsigned int> extendIndices(std::vector<unsigned int> indices1, std::vector<unsigned int> indices2, unsigned int vertext_count) {
    printf("Indices, list1 size %ld, list2 size %ld, vertext count %d\n", indices1.size(), indices2.size(), vertext_count);
    for (unsigned int i = 0; i < indices2.size(); i++) {
        indices1.push_back(indices2[i] + vertext_count);
    }
    return indices1;
}

Mesh combineMeshes(std::vector<Mesh> meshes) {
    Mesh m; 
    for (unsigned int i = 0; i < meshes.size(); i++) {
        Mesh currentMesh = meshes[i];

        m.indices = extendIndices(m.indices, currentMesh.indices, m.vertices.size());
        m.vertices = extendVector(m.vertices, currentMesh.vertices);
        m.textureCoordinates = extendVector(m.textureCoordinates, currentMesh.textureCoordinates);
        m.colours = extendVector(m.colours, currentMesh.colours);
        m.normals = extendVector(m.normals, currentMesh.normals);
    }
    return m;

}


Mesh generateTower(glm::vec3 dimensions) {
    std::vector<Mesh> meshes;

    glm::vec3 centering = glm::vec3(-dimensions.x / 2.0, 0, -dimensions.z /2.0);

    glm::vec3 pillar_scale = glm::vec3(1.0, 1.0, 1.0);

    meshes.push_back(cube_with_offset(dimensions, centering, glm::vec3(1.0, 1.0, 1.0)));
    meshes.push_back(cube_with_offset(glm::vec3(1.0f, 5.0f, 1.0f), centering + glm::vec3(0, dimensions.y, 0), pillar_scale));
    meshes.push_back(cube_with_offset(glm::vec3(1.0f, 5.0f, 1.0f), centering + glm::vec3(dimensions.x - 1, dimensions.y, 0), pillar_scale));
    meshes.push_back(cube_with_offset(glm::vec3(1.0f, 5.0f, 1.0f), centering + glm::vec3(0, dimensions.y, dimensions.z - 1), pillar_scale));
    meshes.push_back(cube_with_offset(glm::vec3(1.0f, 5.0f, 1.0f), centering + glm::vec3(dimensions.x - 1, dimensions.y, dimensions.z - 1), pillar_scale));
    meshes.push_back(cube_with_offset(glm::vec3(dimensions.x, 2, dimensions.z), centering + glm::vec3(0, dimensions.y + 5, 0), pillar_scale));
    Mesh final_mesh = combineMeshes(meshes);

    return final_mesh;
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

Mesh generateFloor(glm::vec2 scale) {
    Mesh m;

    float height_scale = 30.0f;

    SimplexNoise simplex = SimplexNoise();

    int no_horizontal_sections = (int) ((scale.x) / floor_horizontal_section);
    int no_depth_sections = (int) ((scale.y) / floor_depth_section);

    for (int a = 0; a < no_depth_sections; a++) {
        for (int b = 0; b < no_horizontal_sections; b++) {
            long int x_position = b*floor_horizontal_section;
            long int z_position = -1*a*floor_depth_section;

            float noise = simplex.fractal(4, -1*x_position*0.005, z_position*0.005);
            m.vertices.push_back(glm::vec3(x_position, height_scale*noise, z_position));
            m.colours.push_back(getColourFromNoise(noise));
            m.textureCoordinates.push_back({x_position / scale.x, (-1*z_position) / scale.y});
        }
    }
    
    long int offset = 0;
    long int center_num;
    glm::vec3 center;
    glm::vec3 bottom_left;
    glm::vec3 bottom_center;
    glm::vec3 bottom_right;
    glm::vec3 center_left;
    glm::vec3 center_right;
    glm::vec3 top_left;
    glm::vec3 top_center;
    glm::vec3 top_right;

    for (int a = 0; a < no_depth_sections; a++) {
        offset = a*no_horizontal_sections;
        for (int b = 0; b < no_horizontal_sections; b++) {
            //8 possible normals for each and every vertex 
            // 6  7  8
            // 4  x  5
            // 1  2  3
            //Start in bottom left quadrant and move anti-clockwise
            
            center_num = offset + b;
            center = m.vertices[center_num];
            bottom_left = m.vertices[std::max(center_num - no_horizontal_sections - 1, (long int) 0)];
            bottom_center = m.vertices[std::max(center_num - no_horizontal_sections, (long int) 0)];
            bottom_right = m.vertices[std::max(center_num - no_horizontal_sections + 1, (long int) 0)];
            center_left = m.vertices[std::max(center_num - 1, (long int) 0)];
            center_right = m.vertices[std::max(center_num + 1, (long int) 0)];
            top_left = m.vertices[std::max(center_num + no_horizontal_sections - 1, (long int) 0)];
            top_center = m.vertices[std::max(center_num + no_horizontal_sections, (long int) 0)];
            top_right = m.vertices[std::max(center_num + no_horizontal_sections + 1, (long int) 0)];

            glm::vec3 normal = glm::vec3(0.0, 0.0, 0.0);

            normal += a < 1 || b < 1 ? glm::vec3(0.0,0.0,0.0) : (glm::normalize(glm::cross(center - bottom_left, center_left - bottom_left)));
            normal += a < 1 || b < 1 ? glm::vec3(0.0,0.0,0.0) : (glm::normalize(glm::cross(bottom_left - center, bottom_center - center)));

            normal += a < 1 || b == no_horizontal_sections - 1 ? glm::vec3(0.0,0.0,0.0) : (glm::normalize(glm::cross(center_right - bottom_center, center - bottom_center)));
            normal += a < 1 || b == no_horizontal_sections - 1 ? glm::vec3(0.0,0.0,0.0) : (glm::normalize(glm::cross(bottom_center - center_right, bottom_right - center_right)));

            normal += a == no_depth_sections - 1 || b == no_horizontal_sections - 1 ? glm::vec3(0.0,0.0,0.0) : (glm::normalize(glm::cross(top_right - center, top_center - center)));
            normal += a == no_depth_sections - 1 || b == no_horizontal_sections - 1 ? glm::vec3(0.0,0.0,0.0) : (glm::normalize(glm::cross(center - top_right, center_right - top_right)));
            
            normal += a == no_depth_sections - 1 || b < 1 ? glm::vec3(0.0,0.0,0.0) : (glm::normalize(glm::cross(top_center - center_left, top_left - center_left)));
            normal += a == no_depth_sections - 1 || b < 1 ? glm::vec3(0.0,0.0,0.0) : (glm::normalize(glm::cross(center_left - top_center, center - top_center)));

            normal = glm::normalize(normal);
            m.normals.push_back(normal);
        }
    }

    for (int a = 0; a < no_depth_sections-1; a++) {
        offset = a*no_horizontal_sections;
        for (int b = 0; b < no_horizontal_sections-1; b++) {
            int bottom_left = offset + b;
            int top_right = bottom_left + no_horizontal_sections + 1;
            int top_left = bottom_left + no_horizontal_sections;
            int bottom_right = bottom_left + 1;
            
            m.indices.push_back(bottom_left);
            m.indices.push_back(top_right);
            m.indices.push_back(top_left);

            m.indices.push_back(bottom_left);
            m.indices.push_back(bottom_right);
            m.indices.push_back(top_right);
        }
    }

    printf("Offset: %ld ", offset);
    printf("Floor finished. Vertices: %ld, Indices: %ld, Texture: %ld, normals: %ld\n", m.vertices.size(), m.indices.size(), m.textureCoordinates.size(), m.normals.size());
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
