#include "ObjectHandler.h"

void ObjectHandler::loadOBJ(const std::string& Name, const std::string& FilePath) {
    std::ifstream file(FilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + FilePath);
    }

    m_geometrys[Name] = std::make_shared<Geometry>();

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

void ObjectHandler::makeGeometryBuffers(const std::string& Name) {
    glGenVertexArrays(1, &m_geometrys[Name]->VAO);

    glBindVertexArray(m_geometrys[Name]->VAO);

    glGenBuffers(1, &m_geometrys[Name]->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_geometrys[Name]->VBO);
    glBufferData(GL_ARRAY_BUFFER, m_geometrys[Name]->vertecies.size() * sizeof(Vertex), m_geometrys[Name]->vertecies.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_geometrys[Name]->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_geometrys[Name]->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_geometrys[Name]->indecies.size() * sizeof(unsigned int), m_geometrys[Name]->indecies.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

ObjectHandler::~ObjectHandler() {
    for (auto& it : m_geometrys) {
        glDeleteVertexArrays(1, &it.second->VAO);
        glDeleteBuffers(1, &it.second->VBO);
        glDeleteBuffers(1, &it.second->EBO);
    }

    for (auto& it : m_textures) {
        glDeleteTextures(1, &it.second->texture);
    }
}

void ObjectHandler::addMaterial(std::string Name, glm::vec4 Diffuse, glm::vec3 Fresnel, float Shininess) {
    m_materials[Name] = std::make_shared<Material>(Diffuse, Fresnel, Shininess);
}

void ObjectHandler::addMaterial(std::string Name, std::string FilePath) {
    //TODO
}

void ObjectHandler::addTexture(std::string Name, std::string FilePath) {
    m_textures[Name] = std::make_shared<Texture>();
    glGenTextures(1, &m_textures[Name]->texture);
    glBindTexture(GL_TEXTURE_2D, m_textures[Name]->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLfloat value, max_anisotropy = 16.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, & value);

    value = (value > max_anisotropy) ? max_anisotropy : value;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(FilePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
}

void ObjectHandler::addGeometry(const std::string& Name, const std::string& FilePath) {
    if (m_geometrys.find(Name) != m_geometrys.end()) {
        throw std::runtime_error("Geometry already exists in geometries");
    }

    loadOBJ(Name, FilePath);
    makeGeometryBuffers(Name);
}

void ObjectHandler::setObjectType(std::string Name, ObjectType Type) {
    m_renderObjectsMap[Name]->Type = Type;
    m_renderObjectsMapType[m_renderObjectsMap[Name]->Type].push_back(m_renderObjectsMap[Name]);
}

void ObjectHandler::bindObject(std::string Name, std::string GeometryName, std::string TextureName, std::string MaterialName) {
    RenderObject tempRednderObj;
    tempRednderObj.Name = Name;
    tempRednderObj.geometry = m_geometrys[GeometryName];
    tempRednderObj.material = m_materials[MaterialName];

    if (TextureName != "") {
        tempRednderObj.texture = m_textures[TextureName];
    }

    tempRednderObj.objectID = m_renderObjectsVector.size();
    m_renderObjectsMap[Name] = std::make_shared<RenderObject>(tempRednderObj);
    m_renderObjectsVector.push_back(m_renderObjectsMap[Name]);
}

void ObjectHandler::addObjectInstance(std::string Name, const WorldData& world) {
    m_renderObjectsMap[Name]->worldData->push_back(world);
}

std::shared_ptr<RenderObject> ObjectHandler::getObject(std::string Name) {
    return m_renderObjectsMap[Name];
}

std::map<std::string, std::shared_ptr<RenderObject>> ObjectHandler::getObjectsMap() {
    return m_renderObjectsMap;
}

std::vector<std::shared_ptr<RenderObject>> ObjectHandler::getObjectsVector() {
    return m_renderObjectsVector;
}
