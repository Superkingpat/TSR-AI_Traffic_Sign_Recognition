#pragma once
#include "ShaderHandler.h"

struct Material {
    std::string Name;
    glm::vec4 Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
    glm::vec3 Fresnel = glm::vec3(0.5f, 0.5f, 0.5f);
    float Shininess = 0.5f;

    Material() = default;

    Material(const std::string& name, const glm::vec4& diffuse, const glm::vec3& fresnel, float shininess)
        : Name(name), Diffuse(diffuse), Fresnel(fresnel), Shininess(shininess) {
    }
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && uv == other.uv;
    }
};

struct Geometry {
    std::string Name;
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    std::vector<unsigned int> indecies;
    std::vector<Vertex> vertecies;

    Geometry() = default;
};

struct WorldData {
    glm::vec3 Position;
    glm::vec3 Rotation;
    glm::vec3 Scale;
    glm::mat4x4 worldTransform;
    glm::mat4x4 pickedTransform;
};

struct Texture {
    std::string Name;
    unsigned char* data;
    int width, height, nrChannels;
    GLuint texture;
};

struct RenderObject {
    std::string Name;
    std::shared_ptr<Material> material;
    std::shared_ptr<Geometry> geometry;
    std::shared_ptr<Texture> texture;
    WorldData worldData;
    bool Picked;
};

class ObjectHandler
{
};

