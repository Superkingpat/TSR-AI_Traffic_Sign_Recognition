#pragma once
#include "ShaderHandler.h"

struct Material {
    glm::vec4 Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
    glm::vec3 Fresnel = glm::vec3(0.5f, 0.5f, 0.5f);
    float Shininess = 0.5f;

    Material() = default;

    Material(const glm::vec4& diffuse, const glm::vec3& fresnel, float shininess)
        : Diffuse(diffuse), Fresnel(fresnel), Shininess(shininess) {
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

class ObjectHandler {
private:
    std::map<std::string, std::shared_ptr<Material>> m_materials;
    std::map<std::string, std::shared_ptr<Geometry>> m_geometrys;
    std::map<std::string, std::shared_ptr<Texture>> m_textures;
    std::map<std::string, RenderObject> m_renderObjects;

    void loadOBJ(const std::string& Name, const std::string& FilePath);
    void makeGeometryBuffers(const std::string& Name);
public:
    ObjectHandler() = default;
    ~ObjectHandler();

    void addMaterial(std::string Name, glm::vec4 Diffuse, glm::vec3 Fresnel, float Shininess);
    void addMaterial(std::string Name, std::string FilePath);
    void addTexture(std::string Name, std::string FilePath);
    void addGeometry(const std::string& Name, const std::string& FilePath);
    void bindObject(std::string Name, std::string GeometryName, std::string TextureName, std::string MaterialName);
    void setObjectWorld(std::string Name, const WorldData& world);
    RenderObject getObject(std::string Name);
};