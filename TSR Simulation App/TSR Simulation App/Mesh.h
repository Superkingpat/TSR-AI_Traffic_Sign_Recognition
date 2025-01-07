#pragma once

#include "ShaderHandler.h"

enum class TextureType {
    TEXTURE_DIFFUSE,
    TEXTURE_SPECULAR
};

struct Vertex_t {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture_t {
    GLuint id;
    TextureType type;
};

class Mesh {
private:
    std::vector<Vertex_t> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<Texture_t> m_textures;
    unsigned int M_VAO, M_VBO, M_EBO;
    void Init();

public:
    Mesh(const std::vector<Vertex_t>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture_t>& textures);
    ~Mesh();
};

