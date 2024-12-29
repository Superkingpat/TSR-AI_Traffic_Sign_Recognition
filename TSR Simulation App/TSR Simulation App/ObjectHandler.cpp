#include "ObjectHandler.h"

void ObjectHandler::loadOBJ(const std::string& Name, const std::string& FilePath) {
    std::ifstream file(FilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + FilePath);
    }

    m_geometrys[Name] = std::make_shared<Geometry>();
    m_geometrys[Name]->Name = Name;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    struct VertexHash {
        size_t operator()(const Vertex& vertex) const {
            size_t h1 = std::hash<float>{}(vertex.position.x) ^ std::hash<float>{}(vertex.position.y) ^ std::hash<float>{}(vertex.position.z);
            size_t h2 = std::hash<float>{}(vertex.normal.x) ^ std::hash<float>{}(vertex.normal.y) ^ std::hash<float>{}(vertex.normal.z);
            size_t h3 = std::hash<float>{}(vertex.uv.x) ^ std::hash<float>{}(vertex.uv.y);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    std::unordered_map<Vertex, unsigned int, VertexHash> uniqueVertices;
    std::vector<unsigned int> indices;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 position;
            ss >> position.x >> position.y >> position.z;
            positions.push_back(position);
        }
        else if (prefix == "vt") {
            glm::vec2 texCoord;
            ss >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (prefix == "f") {
            std::string vertexData;
            while (ss >> vertexData) {
                std::istringstream vertexStream(vertexData);
                unsigned int posIdx = 0, texIdx = 0, normIdx = 0;
                char slash1 = '\0', slash2 = '\0';

                if (vertexData.find("//") != std::string::npos) {
                    vertexStream >> posIdx >> slash1 >> slash2 >> normIdx;
                }
                else if (vertexData.find('/') != std::string::npos) {
                    vertexStream >> posIdx >> slash1 >> texIdx >> slash2 >> normIdx;
                }
                else {
                    vertexStream >> posIdx;
                }

                Vertex vertex = {
                    positions[posIdx - 1],
                    (normIdx > 0 ? normals[normIdx - 1] : glm::vec3(0.0f)),
                    (texIdx > 0 ? texCoords[texIdx - 1] : glm::vec2(0.0f))
                };

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<unsigned int>(m_geometrys[Name]->vertecies.size());
                    m_geometrys[Name]->vertecies.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
    for (const auto& vertex : m_geometrys[Name]->vertecies) {
        minPos = glm::min(minPos, vertex.position);
        maxPos = glm::max(maxPos, vertex.position);
    }

    glm::vec3 center = (minPos + maxPos) * 0.5f;
    float maxDimension = std::max({ maxPos.x - minPos.x, maxPos.y - minPos.y, maxPos.z - minPos.z });
    for (auto& vertex : m_geometrys[Name]->vertecies) {
        vertex.position = (vertex.position - center) / maxDimension;
    }

    m_geometrys[Name]->indecies = std::move(indices);

    std::cout << "Vertices: " << m_geometrys[Name]->vertecies.size()
        << ", Indices: " << m_geometrys[Name]->indecies.size() << "\n";
}