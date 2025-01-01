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
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
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
    bool Picked;

    glm::mat4x4 getWorldTransform() {
        glm::mat4x4 scaleMatrix = glm::scale(glm::mat4x4(1.0f), Scale);

        glm::mat4x4 rotationMatrix = glm::mat4x4(1.0f);

        if (Rotation[0] > 0.001f)
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Rotation[0]), glm::vec3(1, 0, 0));
        if (Rotation[1] > 0.001f)
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Rotation[1]), glm::vec3(0, 1, 0));
        if (Rotation[2] > 0.001f)
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Rotation[2]), glm::vec3(0, 0, 1));

        glm::mat4x4 translationMatrix = glm::translate(glm::mat4x4(1.0f), Position);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    glm::mat4x4 getPickedTransform() {
        glm::mat4x4 scaleMatrix = glm::scale(glm::mat4x4(1.0f), Scale * 1.02f);

        glm::mat4x4 rotationMatrix = glm::mat4x4(1.0f);

        if (Rotation[0] > 0.001f)
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Rotation[0]), glm::vec3(1, 0, 0));
        if (Rotation[1] > 0.001f)
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Rotation[1]), glm::vec3(0, 1, 0));
        if (Rotation[2] > 0.001f)
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Rotation[2]), glm::vec3(0, 0, 1));

        glm::mat4x4 translationMatrix = glm::translate(glm::mat4x4(1.0f), Position);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    void move(float x, float y, float z) {
        Position = glm::vec3(Position.x + x, Position.y + y, Position.z + z);
    }

    void setPosition(float x, float y, float z) {
        Position = glm::vec3(x, y, z);
    }
};

struct Texture {
    unsigned char* data;
    int width, height, nrChannels;
    GLuint texture;
};

struct RenderObject {
    std::string Name;
    uint32_t objectID;
    std::shared_ptr<Material> material;
    std::shared_ptr<Geometry> geometry;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<std::vector<WorldData>> worldData;

    RenderObject() : worldData(std::make_shared<std::vector<WorldData>>()) {}
};

//TODO
//Find better way to manage the pointers.shared_ptrs are good for memory safty but not for performance
class ObjectHandler {
private:
    std::map<std::string, std::shared_ptr<Material>> m_materials;
    std::map<std::string, std::shared_ptr<Geometry>> m_geometrys;
    std::map<std::string, std::shared_ptr<Texture>> m_textures;
    std::map<std::string, std::shared_ptr<RenderObject>> m_renderObjectsMap;
    std::vector<std::shared_ptr<RenderObject>> m_renderObjectsVector;

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
    void addObjectInstance(std::string Name, const WorldData& world);
    std::shared_ptr<RenderObject> getObject(std::string Name);
    std::map<std::string, std::shared_ptr<RenderObject>> getObjectsMap();
    std::vector<std::shared_ptr<RenderObject>> getObjectsVector();
};