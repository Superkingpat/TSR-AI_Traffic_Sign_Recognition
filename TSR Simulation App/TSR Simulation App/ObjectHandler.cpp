#include "ObjectHandler.h"

ObjectHandler::~ObjectHandler() {
    for (auto& it : m_renderObjectsVector) {
        for (int i = 0; i < it->geometry->size(); i++) {
            glDeleteVertexArrays(1, &it->geometry->at(i).VAO);
            glDeleteBuffers(1, &it->geometry->at(i).VBO);
            glDeleteBuffers(1, &it->geometry->at(i).EBO);
        }

        for(int i = 0; i < it->geometry->size(); i++) {
            glDeleteTextures(1, &it->geometry->at(i).texture.texture);
        }
    }
}

void ObjectHandler::loadOBJ(const std::string& Name, const std::string& FilePath, ObjectType type) {

    RenderObject obj;
    std::vector<Geometry> geo;
    std::vector<Material> mat;

    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(FilePath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    normalizeModelSize(scene, 1.f);

    makeGeometry(scene->mRootNode, scene, geo, mat);

    std::cout << "\n\n\n\n";

    std::cout << "Geo info: " << geo.size() << "\n";
    std::cout << "mat info: " << mat.size() << "\n";
    //std::cout << "tex info: " << tex.texture << "\n";

    obj.Type = type;
    obj.objectID = m_renderObjectsVector.size();
    obj.Name = Name;
    obj.geometry = std::make_shared<std::vector<Geometry>>(geo);
    obj.material = std::make_shared < std::vector<Material>>(mat);

    m_renderObjectsMap[Name] = std::make_shared<RenderObject>(obj);
    m_renderObjectsVector.push_back(m_renderObjectsMap[Name]);
    m_renderObjectsMapType[type].push_back(m_renderObjectsMap[Name]);
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

std::vector<std::shared_ptr<RenderObject>> ObjectHandler::getObjectsVectorType(ObjectType Type) {
    return m_renderObjectsMapType[Type];
}

void ObjectHandler::addObjectInstance(std::string Name, const WorldData& world) {
    m_renderObjectsMap[Name]->worldData->push_back(world);
}

void ObjectHandler::makeGeometry(aiNode* node, const aiScene* scene, std::vector<Geometry>& geo, std::vector<Material>& mat) {
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processGeometry(mesh, scene, geo, mat);
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        makeGeometry(node->mChildren[i], scene, geo, mat);
    }
}

void ObjectHandler::processGeometry(aiMesh* mesh, const aiScene* scene, std::vector<Geometry>& geo, std::vector<Material>& mat) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.uv = vec;
        } else {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    makeGeoBuffers(geo, vertices, indices);

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        Material tempMat;

        aiColor3D co(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, co);
        std::cout << "DIFF: " << co.r << " " << co.g << " " << co.b << "\n";

        glm::vec4 diffVec;
        diffVec.r = co.r;
        diffVec.g = co.g;
        diffVec.b = co.b;
        diffVec.a = 1.f;

        /*if (diffVec.r <= 0.001f && diffVec.g <= 0.001f && diffVec.b <= 0.001f) {
            tempMat.Diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
        } else {
            tempMat.Diffuse = diffVec;
        }*/
        tempMat.Diffuse = diffVec;

        material->Get(AI_MATKEY_COLOR_SPECULAR, co);
        std::cout << "SPEC: " << co.r << " " << co.g << " " << co.b << "\n\n";

        diffVec.r = co.r;
        diffVec.g = co.g;
        diffVec.b = co.b;
        tempMat.Fresnel = diffVec;

        /*if (diffVec.r <= 0.001f && diffVec.g <= 0.001f && diffVec.b <= 0.001f) {
            tempMat.Fresnel = glm::vec3(tempMat.Diffuse.r * 2.f, tempMat.Diffuse.g * 2.f, tempMat.Diffuse.b * 2.f);
        } else {
            tempMat.Fresnel = diffVec;
        }*/
        tempMat.Fresnel = diffVec;

        float shininess = 0.0f;
        if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            float roughness = 1.0f - sqrt(shininess / 1000.0f);
        }

        if (shininess < 0.01f) {
            tempMat.Shininess = 0.f;
        } else {
            tempMat.Shininess = shininess;
        }

        mat.push_back(tempMat);

        if (!geo[geo.size() - 1].texture.used) {
            loadTexture(material, geo);
        }
    }
}

void ObjectHandler::loadTexture(aiMaterial* mat, std::vector<Geometry>& geo) {
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
        return;
    }
    Texture tex;

    aiString str;
    mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);

    glGenTextures(1, &tex.texture);
    glBindTexture(GL_TEXTURE_2D, tex.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLfloat value, max_anisotropy = 16.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);

    value = (value > max_anisotropy) ? max_anisotropy : value;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(("Models/" + std::string(str.C_Str())).c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
        tex.used = false;
        return;
    }

    stbi_image_free(data);

    tex.used = true;
    geo[geo.size() - 1].texture = tex;
}

void ObjectHandler::makeGeoBuffers(std::vector<Geometry>& geo, std::vector<Vertex>& vertecies, std::vector<unsigned int>& indecies) {
    Geometry tempGeo;
    tempGeo.size = indecies.size();
    glGenVertexArrays(1, &tempGeo.VAO);

    glBindVertexArray(tempGeo.VAO);

    glGenBuffers(1, &tempGeo.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, tempGeo.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertecies.size() * sizeof(Vertex), vertecies.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &tempGeo.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempGeo.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indecies.size() * sizeof(unsigned int), indecies.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    geo.push_back(tempGeo);
}

void ObjectHandler::normalizeModelSize(const aiScene* scene, float desiredSize) {
    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(std::numeric_limits<float>::lowest());

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];

        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            aiVector3D vertex = mesh->mVertices[v];
            min.x = std::min(min.x, vertex.x);
            min.y = std::min(min.y, vertex.y);
            min.z = std::min(min.z, vertex.z);

            max.x = std::max(max.x, vertex.x);
            max.y = std::max(max.y, vertex.y);
            max.z = std::max(max.z, vertex.z);
        }
    }

    glm::vec3 size = max - min;
    float maxExtent = std::max(size.x, std::max(size.y, size.z));
    float scaleFactor = desiredSize / maxExtent;

    glm::vec3 center = (max + min) / 2.0f;

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];

        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            aiVector3D& vertex = mesh->mVertices[v];

            vertex.x = (vertex.x - center.x) * scaleFactor;
            vertex.y = (vertex.y - center.y) * scaleFactor;
            vertex.z = (vertex.z - center.z) * scaleFactor;
        }
    }
}
