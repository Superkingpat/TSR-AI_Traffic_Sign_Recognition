#include "ObjectHandler.h"

ObjectHandler::~ObjectHandler() {
    for (auto& it : m_renderObjectsVector) {

        glDeleteVertexArrays(1, &it->geometry.VAO);
        glDeleteBuffers(1, &it->geometry.VBO);
        glDeleteBuffers(1, &it->geometry.EBO);
        for(int i = 0; i < it->geometry.textures.size(); i++) {
            glDeleteTextures(1, &it->geometry.textures[i].texture);
        }
    }
}

void ObjectHandler::loadOBJ(const std::string& Name, const std::string& FilePath, ObjectType type, bool waterApplied) {

    RenderObject obj;
    Geometry geo;
    std::vector<Material> mat;

    std::vector<unsigned int> startIndexies;
    std::vector<unsigned int> numOfIndecies;

    std::vector<std::vector<unsigned int>> indexies;
    std::vector<std::vector<Vertex>> vertecies;

    std::vector<Vertex> flattenedVertices;
    std::vector<unsigned int> flattenedIndices;

    std::vector<Texture> textures;

    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(FilePath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    normalizeModelSize(scene, 1.f);

    makeGeometry(scene->mRootNode, scene, vertecies, indexies, mat, textures,Name,waterApplied);

    std::cout << "\n\n\n\n";

    //std::cout << "Geo info: " << geo.size() << "\n";
    std::cout << "mat info: " << mat.size() << "\n";
    //std::cout << "tex info: " << tex.texture << "\n";

    unsigned int vertexOffset = 0;

    for (size_t i = 0; i < vertecies.size(); ++i) {
        flattenedVertices.insert(flattenedVertices.end(), vertecies[i].begin(), vertecies[i].end());

        for (unsigned int index : indexies[i]) {
            flattenedIndices.push_back(index + vertexOffset);
        }

        startIndexies.push_back(vertexOffset);
        numOfIndecies.push_back(vertecies[i].size());
        vertexOffset += vertecies[i].size();
    }

    makeGeoBuffers(geo, flattenedVertices, flattenedIndices);
    geo.startIndexies = startIndexies;
    geo.numOfIndecies = numOfIndecies;
    geo.textures = textures;

    if (waterApplied) geo.waterApplied = true;

    obj.Type = type;
    obj.objectID = m_renderObjectsVector.size();
    obj.Name = Name;
    obj.geometry = geo;
    obj.material = mat;

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
    m_renderObjectsMap[Name]->worldData.push_back(world);
}

void ObjectHandler::makeGeometry(aiNode* node, const aiScene* scene, std::vector<std::vector<Vertex>>& vertecies, std::vector<std::vector<unsigned int>>& indexies, std::vector<Material>& mat, std::vector<Texture>& textures, const std::string& objName, bool waterApplied) {
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processGeometry(mesh, scene, vertecies, indexies, mat, textures,objName,waterApplied);
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        makeGeometry(node->mChildren[i], scene, vertecies, indexies, mat, textures, objName, waterApplied);
    }
}

void ObjectHandler::processGeometry(aiMesh* mesh, const aiScene* scene, std::vector<std::vector<Vertex>>& vertecies, std::vector<std::vector<unsigned int>>& indexies, std::vector<Material>& mat, std::vector<Texture>& textures, const std::string& objName, bool waterApplied) {
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

    //makeGeoBuffers(geo, vertices, indices);
    vertecies.push_back(vertices);
    indexies.push_back(indices);

    Texture tex;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        Material tempMat;

        aiColor3D co(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, co);
        std::cout << "DIFF: " << co.r << " " << co.g << " " << co.b << "\n";

        float alpha = 1.0f;

        if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY, alpha)) {
            std::cout << "ALPHA: " << alpha << "\n";
        }

        glm::vec4 diffVec;
        diffVec.r = co.r;
        diffVec.g = co.g;
        diffVec.b = co.b;
        diffVec.a = alpha;

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

        if (diffVec.r <= 0.001f && diffVec.g <= 0.001f && diffVec.b <= 0.001f) {
            tempMat.Fresnel = glm::vec3(0.1f, 0.1f, 0.1f);
        } else {
            tempMat.Fresnel = diffVec;
        }
        //tempMat.Fresnel = diffVec;

        float shininess = 0.0f;
        if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            float roughness = 1.0f - sqrt(shininess / 1000.0f);
            roughness = shininess;
        }

        if (shininess < 0.01f) {
            tempMat.Shininess = 0.01f;
        } else {
            tempMat.Shininess = shininess;
        }

        mat.push_back(tempMat);

        if (!tex.used) {
            loadTexture(material, tex,objName,waterApplied);
        }

        if (tex.used) {
            textures.push_back(tex);
        }
    }
}


void ObjectHandler::loadTexture(aiMaterial* mat, Texture& texture, const std::string& objName, bool waterApplied) {
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
        return;
    }

    Texture tex;

    aiString str;
    mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
    std::cout << "Texture:" << mat->GetTexture(aiTextureType_DIFFUSE, 0, &str) << std::endl;
    std::cout << "Loading texture for .obj: " << objName << std::endl;
    std::string texturePath = "Models/" + std::string(str.C_Str());
    std::cout << "Attempting to load texture: " << texturePath << std::endl;


    glGenTextures(1, &tex.texture);
    glBindTexture(GL_TEXTURE_2D, tex.texture);
    std::cout << "Texture:" << &tex.texture << std::endl;

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    GLfloat value, max_anisotropy = 16.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);

    value = (value > max_anisotropy) ? max_anisotropy : value;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);

    int width, height, nrChannels;
    unsigned char* data;
    if (waterApplied) {
        // Store both textures in the same texture object using different texture units
        glActiveTexture(GL_TEXTURE0);
        unsigned char* data1 = stbi_load("Models/water-natural-1.jpg", &width, &height, &nrChannels, 0);
        if (data1) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                (nrChannels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data1);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data1);
            tex.used = true;
        }

        // Store the second texture in a new texture object
        GLuint texture2;
        glGenTextures(1, &texture2);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // Set parameters for second texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        unsigned char* data2 = stbi_load("Models/water-natural-2.jpg", &width, &height, &nrChannels, 0);
        if (data2) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                (nrChannels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data2);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data2);

            // Store the second texture ID in the texture object
            tex.texture2 = texture2;
            tex.isMultiTexture = true;
        }

        std::cout << "Cooking" << std::endl;
    }
    else {
        // Load regular texture
        aiString str;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        std::string texturePath = "Models/" + std::string(str.C_Str());

        unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                (nrChannels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
            tex.used = true;
        }
    }

    // Reset active texture unit
    glActiveTexture(GL_TEXTURE0);
    texture = tex;

    /*
    if (waterApplied) {
        std::cout << "Water:" << waterApplied << std::endl;

        std::string texturePath1 = "Models/water-natural-1.jpg";
        data = stbi_load(texturePath1.c_str(), &width, &height, &nrChannels, 0);
    } else {
        data = stbi_load(("Models/" + std::string(str.C_Str())).c_str(), &width, &height, &nrChannels, 0);
    }

    //data = stbi_load(("Models/" + std::string(str.C_Str())).c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        if (nrChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    } else {
        std::cout << "Failed to load texture" << std::endl;
        tex.used = false;
        return;
    }

    stbi_image_free(data);
    */


}

void ObjectHandler::makeGeoBuffers(Geometry& geo, std::vector<Vertex>& vertecies, std::vector<unsigned int>& indecies) {
    Geometry tempGeo;
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

    geo = tempGeo;
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
