#pragma once

#include "Timer.h"

class Waves
{
public:
    Waves(int m, int n, float dx, float dt, float speed, float damping);
    Waves(const Waves& rhs) = delete;
    Waves& operator=(const Waves& rhs) = delete;
    ~Waves();

    int RowCount() const;
    int ColumnCount() const;
    int VertexCount() const;
    int TriangleCount() const;
    float Width() const;
    float Depth() const;

    const glm::vec3& Position(int i) const { return m_currSolution[i]; }

    const glm::vec3& Normal(int i) const { return m_normals[i]; }

    void Update(float dt);
    void Disturb(int i, int j, float magnitude);

private:
    int m_numRows = 0;
    int m_numCols = 0;

    int m_vertexCount = 0;
    int m_triangleCount = 0;

    float m_K1 = 0.0f;
    float m_K2 = 0.0f;
    float m_K3 = 0.0f;

    float m_timeStep = 0.0f;
    float m_spatialStep = 0.0f;

    float t = 0;

    std::vector<glm::vec3> m_prevSolution;
    std::vector<glm::vec3> m_currSolution;
    std::vector<glm::vec3> m_normals;
};

