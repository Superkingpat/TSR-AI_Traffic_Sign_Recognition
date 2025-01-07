#include "Mesh.h"

void Mesh::Init() {
    glGenVertexArrays(1, &M_VAO);
    glGenBuffers(1, &M_VBO);
    glGenBuffers(1, &M_EBO);

    glBindVertexArray(M_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, M_VBO);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex_t), &m_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, M_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
        &m_indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)offsetof(Vertex_t, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)offsetof(Vertex_t, TexCoords));

    glBindVertexArray(0);
}

Mesh::Mesh(const std::vector<Vertex_t>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture_t>& textures) {
    m_vertices = vertices;
    m_indices = indices;
    m_textures = textures;

    Init();
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &M_VAO);
    glDeleteBuffers(1, &M_VBO);
    glDeleteBuffers(1, &M_EBO);
}
