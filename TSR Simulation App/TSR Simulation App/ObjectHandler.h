#pragma once
#include "ShaderHandler.h"

struct TextureCbMp {
    unsigned char* data = nullptr;
    GLuint texture;
};

enum class ObjectType {
    UNDEFIENED,
    TRAFFIC_SIGHN,
    ROAD,
    GRASS,
    CAR,
    OPP_CAR,
    CLUTTER
};

struct Material {
    glm::vec4 Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
    glm::vec3 Fresnel = glm::vec3(0.5f, 0.5f, 0.5f);
    float Shininess = 0.5f;

    Material()
        : Diffuse(0.5f, 0.5f, 0.5f, 1.f), Fresnel(0.5f, 0.5f, 0.5f), Shininess(0.5f) {
    }

    Material(const glm::vec4& diffuse, const glm::vec3& fresnel, float shininess)
        : Diffuse(diffuse), Fresnel(fresnel), Shininess(shininess) {
    }
};

struct Texture {
    GLuint texture = 0;
    bool used = false;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Geometry {
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    std::vector<Texture> textures;
    std::vector<unsigned int> startIndexies;
    std::vector<unsigned int> numOfIndecies;
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

struct RenderObject {
    std::string Name;
    ObjectType Type;
    uint32_t objectID;
    std::vector<WorldData> worldData;
    std::vector<Material> material;
    Geometry geometry;
};

class ObjectHandler {
private:
    std::map<std::string, std::shared_ptr<RenderObject>> m_renderObjectsMap;
    std::vector<std::shared_ptr<RenderObject>> m_renderObjectsVector;
    std::map<ObjectType, std::vector<std::shared_ptr<RenderObject>>> m_renderObjectsMapType;

    void makeGeometry(aiNode* node, const aiScene* scene, std::vector<std::vector<Vertex>>& vertecies, std::vector<std::vector<unsigned int>>& indexies, std::vector<Material>& mat, std::vector<Texture>& textures);
    void processGeometry(aiMesh* mesh, const aiScene* scene, std::vector<std::vector<Vertex>>& vertecies, std::vector<std::vector<unsigned int>>& indexies, std::vector<Material>& mat, std::vector<Texture>& textures);
    void loadTexture(aiMaterial* mat, Texture& texture);
    void makeGeoBuffers(Geometry& geo, std::vector<Vertex>& vertecies, std::vector<unsigned int>& indecies);
    void normalizeModelSize(const aiScene* scene, float desiredSize);
public:
    ObjectHandler() = default;
    ~ObjectHandler();

    void loadOBJ(const std::string& Name, const std::string& FilePath, ObjectType type = ObjectType::CLUTTER);
    std::shared_ptr<RenderObject> getObject(std::string Name);
    std::map<std::string, std::shared_ptr<RenderObject>> getObjectsMap();
    std::vector<std::shared_ptr<RenderObject>> getObjectsVector();
    std::vector<std::shared_ptr<RenderObject>> getObjectsVectorType(ObjectType Type);
    void addObjectInstance(std::string Name, const WorldData& world);
};