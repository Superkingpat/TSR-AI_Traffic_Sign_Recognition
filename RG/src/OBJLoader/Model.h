#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include "../../vendor/assimp/include/assimp/Importer.hpp"
#include "../../vendor/assimp/include/assimp/postprocess.h"
#include "../../vendor/assimp/include/assimp/scene.h"

#include "/Users/lukaovsenjak/Downloads/glm/glm.hpp"
#include <string>
#include <vector>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Color;
};

class Model
{
private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    void loadModel(const char *path);
    void processNode(aiNode *node, const aiScene *scene);
    void processMesh(aiMesh *mesh, const aiScene *scene);
    void setupMesh();

public:
    Model(const char *path)
    {
        loadModel(path);
        setupMesh();
    }
    void Draw(unsigned int shaderProgram);
};

#endif