#include "Water.h"

void Water::Update(float dt) {
	static float t = 0;

	t += dt;

	if (t >= m_timeStep) {
		concurrency::parallel_for(1, m_numRows - 1, [this](int i) {
				for (int j = 1; j < m_numCols - 1; ++j) {

					m_prevSolution[i * m_numCols + j].y =
						m_K1 * m_prevSolution[i * m_numCols + j].y +
						m_K2 * m_currSolution[i * m_numCols + j].y +
						m_K3 * (m_currSolution[(i + 1) * m_numCols + j].y +
							m_currSolution[(i - 1) * m_numCols + j].y +
							m_currSolution[i * m_numCols + j + 1].y +
							m_currSolution[i * m_numCols + j - 1].y);
				}
			});

		std::swap(m_prevSolution, m_currSolution);

		t = 0.0f;

		concurrency::parallel_for(1, m_numRows - 1, [this](int i) {
				for (int j = 1; j < m_numCols - 1; ++j) {
					float l = m_currSolution[i * m_numCols + j - 1].y;
					float r = m_currSolution[i * m_numCols + j + 1].y;
					float t = m_currSolution[(i - 1) * m_numCols + j].y;
					float b = m_currSolution[(i + 1) * m_numCols + j].y;
					m_normals[i * m_numCols + j].x = -r + l;
					m_normals[i * m_numCols + j].y = 2.0f * m_spatialStep;
					m_normals[i * m_numCols + j].z = b - t;

					m_normals[i * m_numCols + j] = glm::normalize(m_normals[i * m_numCols + j]);

					m_tangent[i * m_numCols + j] = glm::vec3(2.0f * m_spatialStep, r - l, 0.0f);
					m_tangent[i * m_numCols + j] = glm::normalize(m_tangent[i * m_numCols + j]);
				}
			});
	}
}

void Water::Disturb(int i, int j, float magnitude) {
	assert(i > 1 && i < m_numRows - 2);
	assert(j > 1 && j < m_numCols - 2);

	float halfMag = 0.5f * magnitude;

	m_currSolution[i * m_numCols + j].y += magnitude;
	m_currSolution[i * m_numCols + j + 1].y += halfMag;
	m_currSolution[i * m_numCols + j - 1].y += halfMag;
	m_currSolution[(i + 1) * m_numCols + j].y += halfMag;
	m_currSolution[(i - 1) * m_numCols + j].y += halfMag;
}

Water::Water(int m, int n, float dx, float dt, float speed, float damping) {
    m_numRows = m;
    m_numCols = n;

    m_vertexCount = m * n;
    m_triangleCount = (m - 1) * (n - 1) * 2;

    m_timeStep = dt;
    m_spatialStep = dx;

    float d = damping * dt + 2.0f;
    float e = (speed * speed) * (dt * dt) / (dx * dx);
    m_K1 = (damping * dt - 2.0f) / d;
    m_K2 = (4.0f - 8.0f * e) / d;
    m_K3 = (2.0f * e) / d;

    m_prevSolution.resize(m * n);
    m_currSolution.resize(m * n);
    m_normals.resize(m * n);
    m_tangent.resize(m * n);

    float halfWidth = (n - 1) * dx * 0.5f;
    float halfDepth = (m - 1) * dx * 0.5f;
    for (int i = 0; i < m; ++i) {
        float z = halfDepth - i * dx;
        for (int j = 0; j < n; ++j) {
            float x = -halfWidth + j * dx;

            m_prevSolution[i * n + j] = glm::vec3(x, 0.0f, z);
            m_currSolution[i * n + j] = glm::vec3(x, 0.0f, z);
            m_normals[i * n + j] = glm::vec3(0.0f, 1.0f, 0.0f);
            m_tangent[i * n + j] = glm::vec3(1.0f, 0.0f, 0.0f);
        }
    }
}

int Water::RowCount() const {
	return m_numRows;
}

int Water::ColumnCount() const {
	return m_numCols;
}

int Water::VertexCount() const {
	return m_vertexCount;
}

int Water::TriangleCount() const {
	return m_triangleCount;
}

float Water::Width() const {
	return m_numCols*m_spatialStep;
}

float Water::Depth() const {
	return m_numRows*m_spatialStep;
}


