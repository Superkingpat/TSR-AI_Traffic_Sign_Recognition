#include "TSR_Simulation.h"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void TSR_Simulation::Init() {
    InitGLFW();
    InitImgui();
    InitCamera();
    InitOpenGL();
    InitRenderObjects();
    InitCubemap();
    InitLights();
    InitWater();
    InitShaders();
    InitRenderedWater();
    m_generationBiases.fill(50);
}

void TSR_Simulation::InitCamera() {
    CameraConfig conf;
    conf.Position = glm::vec3(-4.0f, 3.0f, 3.0f);
    conf.Front = glm::vec3(0.0f, 0.0f, 1.0f);
    conf.Up = glm::vec3(0.0f, 1.0f, 0.0f);
    conf.projection = glm::perspective(glm::radians(45.0f), (float)M_SCR_WIDTH / (float)M_SCR_HEIGHT, 0.01f, 100.f);
    conf.sensitivity = 1.f;
    conf.pitch = 0.f;
    conf.yaw = 0.f;
    conf.speed = 0.f;
    m_cameraHandlerOuter = CameraHandler(conf, glm::vec3(0.f, 0.2f, 0.f), true);

    conf.speed = 0.f;
    conf.sensitivity = 0.f;
    conf.Position = glm::vec3(0.f, 0.22f, 0.25f);
    conf.projection = glm::perspective(glm::radians(45.0f), (float)M_CAR_SCR_WIDTH / (float)M_CAR_SCR_HEIGHT, 0.01f, 100.f);
    m_cameraHandlerInner = CameraHandler(conf);

    conf.speed = 10.f;
    conf.sensitivity = 80.f;
    conf.yaw = 60.7722f;
    conf.pitch = -28.7373f;
    conf.projection = glm::ortho(-40.0f, 20.0f, -10.0f, 15.0f, 1.f, 50.f);
    conf.Front = glm::vec3(0.00348613f, -0.480794f, 0.876827f);
    conf.Position = glm::vec3(10.f, 10.f, -10.f);
    m_cameraHandlerShadow = CameraHandler(conf);
}

void TSR_Simulation::InitGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    M_SCR_WIDTH = mode->width;
    M_SCR_HEIGHT = mode->height;
    M_CAR_SCR_WIDTH = 1920;
    M_CAR_SCR_HEIGHT = 1080;

    m_window = glfwCreateWindow(M_SCR_WIDTH, M_SCR_HEIGHT, "TSR Simulation", /*NULL*/glfwGetPrimaryMonitor(), NULL);

    if (m_window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
}

void TSR_Simulation::InitImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();
}

void TSR_Simulation::InitOpenGL() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0x00);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void TSR_Simulation::InitRenderedWater() {

    Texture tex;
    Texture tex2;

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


    glActiveTexture(GL_TEXTURE0);
    int width, height, nrChannels;
    unsigned char* data1 = stbi_load("Models/water-natural-1.jpg", &width, &height, &nrChannels, 0);
    if (data1) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            (nrChannels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data1);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data1);
        tex.used = true;
    }

    glGenTextures(1, &tex2.texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex2.texture);

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
    }

    Material mat1;
    mat1.Diffuse = glm::vec4(0.2f, 0.4f, 0.8f, 0.8f); // mat1.Diffuse = glm::vec4(0.2f, 0.4f, 0.8f, 0.8f);
    mat1.Fresnel = glm::vec3(0.95f, 0.95f, 0.95f); // mat1.Fresnel = glm::vec3(0.95f, 0.95f, 0.95f);
    mat1.Shininess = 0.9f;

    Geometry geo;

    geo.VAO = m_objectHandler.getObject("grass")->geometry.VAO;

    geo.textures.push_back(tex);
    geo.textures.push_back(tex2);

    buffers.water.geometry = geo;
    buffers.water.material.push_back(mat1);

    WorldData wdata1;
    wdata1.Scale = glm::vec3(4.f, 4.f, 4.f);
    //buffers.water.worldData.push_back(data);
    //data.Position = glm::vec3(1.f, 1.0f, 1.0f);
    for (int i = 0; i < 30; i++) {
        wdata1.Position = glm::vec3(-30.f + i * 3.99f, 0.0f, 0.0f);  // Adjusted position
        wdata1.Position.z = 14.5f;
        buffers.water.worldData.push_back(wdata1);
    }

    WorldData wdata2;
    wdata2.Scale = glm::vec3(4.f, 4.f, 4.f);
    //buffers.water.worldData.push_back(data);
    //data.Position = glm::vec3(1.f, 1.0f, 1.0f);
    for (int i = 0; i < 30; i++) {
        wdata2.Position = glm::vec3(-30.f + i * 3.99f, 0.0f, 0.0f);  // Adjusted position
        wdata2.Position.z = -14.5f;
        buffers.water.worldData.push_back(wdata2);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(0);
}

void TSR_Simulation::InitRenderObjects() {
    WorldData wd;
    wd.Picked = false;
    wd.Scale = glm::vec3(4.f, 4.f, 8.f);
    wd.Position = glm::vec3(-30.f, 0.f, 0.f);
    wd.Rotation = glm::vec3(0.f, 0.f, 0.f);

    m_objectHandler.loadOBJ("road", "Models/road2.obj", ObjectType::ROAD);

    for (int i = 0; i < 30; i++) {
        m_objectHandler.addObjectInstance("road", wd);
        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, 0.f);
        std::cout << "\n RoadX: " << wd.Position.x << "\n";
    }

    m_objectHandler.loadOBJ("grass", "Models/bestgrass.obj", ObjectType::ROAD);

    wd.Scale = glm::vec3(4.f, 4.f, 4.f);

    for (int i = 0; i < 29; i++) {
        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.0f, 0.f);

        wd.Position.z = 2.5f;
        m_objectHandler.addObjectInstance("grass", wd);
        
        wd.Position.z = 6.5f;
        m_objectHandler.addObjectInstance("grass", wd);
        
        wd.Position.z = 10.5f;
        m_objectHandler.addObjectInstance("grass", wd);

        wd.Position.z = -2.5f;
        m_objectHandler.addObjectInstance("grass", wd);
        wd.Position.z = -6.5f;
        m_objectHandler.addObjectInstance("grass", wd);
        wd.Position.z = -10.5f;
        m_objectHandler.addObjectInstance("grass", wd);
        std::cout << "\n GrassX: " << wd.Position.x << "\n";
    }

    m_objectHandler.loadOBJ("20", "Models/20.obj");
    wd.Position = glm::vec3(0.f, 0.3f, 1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("20", wd);

    m_objectHandler.loadOBJ("60", "Models/60.obj");
    wd.Position = glm::vec3(4.f, 0.3f, 1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("60", wd);

    m_objectHandler.loadOBJ("odvzemPrednosti", "Models/odvzemPrednosti.obj");
    wd.Position = glm::vec3(20.f, 0.3f, 1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("odvzemPrednosti", wd);

    m_objectHandler.loadOBJ("stop", "Models/stop.obj");
    wd.Position = glm::vec3(40.f, 0.3f, -1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("stop", wd);

    m_objectHandler.loadOBJ("130", "Models/130.obj");
    wd.Position = glm::vec3(-20.f, 0.3f, -1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("130", wd);

    m_objectHandler.loadOBJ("20-", "Models/20-.obj");
    m_objectHandler.loadOBJ("30", "Models/30.obj");
    m_objectHandler.loadOBJ("30-", "Models/30-.obj");
    m_objectHandler.loadOBJ("40", "Models/40.obj");
    m_objectHandler.loadOBJ("40-", "Models/40-.obj");
    m_objectHandler.loadOBJ("50", "Models/50.obj");
    m_objectHandler.loadOBJ("50-", "Models/50-.obj");
    m_objectHandler.loadOBJ("60-", "Models/60-.obj");
    m_objectHandler.loadOBJ("70", "Models/70.obj");
    m_objectHandler.loadOBJ("70-", "Models/70-.obj");
    m_objectHandler.loadOBJ("80", "Models/80.obj");
    m_objectHandler.loadOBJ("80-", "Models/80-.obj");
    m_objectHandler.loadOBJ("90", "Models/90.obj");
    m_objectHandler.loadOBJ("90-", "Models/90-.obj");
    m_objectHandler.loadOBJ("100", "Models/100.obj");
    m_objectHandler.loadOBJ("100-", "Models/100-.obj");
    m_objectHandler.loadOBJ("110", "Models/110.obj");
    m_objectHandler.loadOBJ("110-", "Models/110-.obj");
    m_objectHandler.loadOBJ("120", "Models/120.obj");
    m_objectHandler.loadOBJ("120-", "Models/120-.obj");
    m_objectHandler.loadOBJ("130-", "Models/130-.obj");
    m_objectHandler.loadOBJ("konecvsehomejitev", "Models/konecvsehomejitev.obj");

    m_objectHandler.loadOBJ("tree", "Models/tree2.obj");
    wd.Scale = glm::vec3(2.f, 2.f, 2.f);
    wd.Position = glm::vec3(10.f, 1.f, 3.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(2.f, 3.f, 2.f);
    wd.Position = glm::vec3(20.f, 1.f, -1.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(2.f, 2.f, 2.f);
    wd.Position = glm::vec3(40.f, 1.f, -6.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(2.f, 4.f, 2.f);
    wd.Position = glm::vec3(-20.f, 1.f, 5.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(3.f, 3.f, 3.f);
    wd.Position = glm::vec3(50.f, 1.f, 6.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(3.f, 4.f, 3.f);
    wd.Position = glm::vec3(20.f, 1.5f, -6.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(3.f, 3.f, 3.f);
    wd.Position = glm::vec3(70.f, 1.f, -6.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(3.f, 2.f, 3.f);
    wd.Position = glm::vec3(60.f, 1.f, 2.f);
    m_objectHandler.addObjectInstance("tree", wd);

    m_objectHandler.loadOBJ("car", "Models/car.obj", ObjectType::CAR);
    m_objectHandler.loadOBJ("carOpp", "Models/car.obj", ObjectType::OPP_CAR);
    wd.Scale = glm::vec3(0.7f, 0.7f, 0.7f);
    wd.Position = glm::vec3(0.f, 0.15f, 0.25f);
    wd.Rotation.y = 0.f;
    m_objectHandler.addObjectInstance("car", wd);

    /*Popravi �e model*/
    m_objectHandler.loadOBJ("carInterior", "Models/carInterior.obj", ObjectType::CAR);
    //wd.Scale = glm::vec3(0.125f, 0.3f, 0.3f); // daj - pravilno
    wd.Scale = glm::vec3(0.125f, 0.3f, 0.3f);
    //wd.Position = glm::vec3(0.05f, 0.15f, 0.3f); // ydaj ga ven dan - gor,dol: nazaj,naprej :levo,desno - pravilno
    wd.Position = glm::vec3(0.12f, 0.15f, 0.25f); // wd.Position = glm::vec3(0.1f, 0.15f, 0.3f);
    wd.Rotation.y = 0.f;
    m_objectHandler.addObjectInstance("carInterior", wd);

    m_pickedRenderObject = m_objectHandler.getObjectsVector()[0];
}

void TSR_Simulation::InitCubemap() {
    InitCubemapBuffers();
    InitCubemapTextures();
}

void TSR_Simulation::InitCubemapTextures() {
    std::vector<std::string> cubemapFaces = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "back.jpg",
        "front.jpg",
    };

    /*std::vector<std::string> cubemapFaces = {
        "posx.jpg",
        "negx.jpg",
        "posy.jpg",
        "negy.jpg",
        "posz.jpg",
        "negz.jpg"
    };*/

    glGenTextures(1, &buffers.cubemapTexture.texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, buffers.cubemapTexture.texture);

    int width, height, nrChannels;

    for (unsigned int i = 0; i < cubemapFaces.size(); i++) {
        buffers.cubemapTexture.data = stbi_load(("Textures/Cubemap/" + cubemapFaces[i]).c_str(), &width, &height, &nrChannels, 0);

        if (buffers.cubemapTexture.data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffers.cubemapTexture.data);
            stbi_image_free(buffers.cubemapTexture.data);
        }
        else {
            std::cout << "Cubemap tex failed to load at path: " << cubemapFaces[i] << "\n";
            stbi_image_free(buffers.cubemapTexture.data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void TSR_Simulation::InitCubemapBuffers() {

    glGenVertexArrays(1, &buffers.cubemapVAO);
    glBindVertexArray(buffers.cubemapVAO);

    glGenBuffers(1, &buffers.cubemapVBO);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.cubemapVBO);

    glBufferData(GL_ARRAY_BUFFER, m_cubemapFaces.size() * sizeof(float), m_cubemapFaces.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void TSR_Simulation::InitLights() {
    m_ambientColor = glm::vec3(0.6f, 0.6f, 0.6f);
    Light lit;
    lit.Strength = glm::vec3(0.4f, 0.3f, 0.3f);
    lit.Direction = glm::vec3(2.f, -3.f, 1.f);
    lit.Type = 1;
    m_lights.push_back(lit);
    /*lit.Direction = glm::vec3(1.f, -3.f, 1.f);
    m_lights.push_back(lit);*/
}

void TSR_Simulation::InitWater() {
    WorldData tempData;
    tempData.Position = glm::vec3(1.f, -1.f, 1.f);
    tempData.Rotation = glm::vec3(0.f, 0.f, 0.f);
    tempData.Scale = glm::vec3(1.f, 1.f, 1.f);

    Material tempMat;
    tempMat.Diffuse = glm::vec4(0.02f, 0.1f, 0.2f, 1.0f);
    tempMat.Fresnel = glm::vec3(0.6f, 0.6f, 0.6f);
    tempMat.Shininess = 0.3f;

    std::vector<unsigned int> indices(3 * m_water.TriangleCount());

    int m = m_water.RowCount();
    int n = m_water.ColumnCount();
    int k = 0;

    for (int i = 0; i < m - 1; ++i) {
        for (int j = 0; j < n - 1; ++j) {
            indices[k] = i * n + j;
            indices[k + 1] = i * n + j + 1;
            indices[k + 2] = (i + 1) * n + j;

            indices[k + 3] = (i + 1) * n + j;
            indices[k + 4] = i * n + j + 1;
            indices[k + 5] = (i + 1) * n + j + 1;

            k += 6;
        }
    }

    buffers.indexCount = indices.size();

    std::vector<Vertex> vers(m_water.VertexCount());

    for (int i = 0; i < m_water.VertexCount(); i++) {
        vers[i] = Vertex();
        vers[i].position = glm::vec3(i, i, i);
    }

    Geometry tempGeo;
    glGenVertexArrays(1, &tempGeo.VAO);

    glBindVertexArray(tempGeo.VAO);

    glGenBuffers(1, &tempGeo.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, tempGeo.VBO);
    glBufferData(GL_ARRAY_BUFFER, vers.size() * sizeof(Vertex), vers.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &tempGeo.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempGeo.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    buffers.waterObject.geometry = tempGeo;
    buffers.waterObject.material.push_back(tempMat);
    buffers.waterObject.worldData.push_back(tempData);
}

void TSR_Simulation::InitBuffers() {
    //Init all neceserry buffers such as frame buffers, texture buffers, depth buffers...
    InitMaterialsBuffers();
    InitLightBuffers();
    InitPickingBuffers();
    InitSecondViewBuffers();
    InitShadowBuffers();
}

void TSR_Simulation::InitMaterialsBuffers() {
    glGenBuffers(1, &buffers.materialUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffers.materialUBO);
}

void TSR_Simulation::InitLightBuffers() {
    glGenBuffers(1, &buffers.lightsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * M_MAX_NUM_OF_LIGHTS, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, buffers.lightsUBO);
}

void TSR_Simulation::InitPickingBuffers() {
    glGenFramebuffers(1, &buffers.pickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, buffers.pickingFBO);

    glGenTextures(1, &buffers.pickingTexture);
    glBindTexture(GL_TEXTURE_2D, buffers.pickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, M_SCR_WIDTH, M_SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenRenderbuffers(1, &buffers.pickingDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, buffers.pickingDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, M_SCR_WIDTH, M_SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffers.pickingDepthRBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffers.pickingTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Error: Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TSR_Simulation::InitSecondViewBuffers() {
    glGenFramebuffers(1, &buffers.secondViewFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, buffers.secondViewFBO);

    glGenTextures(1, &buffers.secondViewTexture);
    glBindTexture(GL_TEXTURE_2D, buffers.secondViewTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, M_CAR_SCR_WIDTH, M_CAR_SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenRenderbuffers(1, &buffers.secondViewDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, buffers.secondViewDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, M_CAR_SCR_WIDTH, M_CAR_SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffers.secondViewDepthRBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffers.secondViewTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Error: Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TSR_Simulation::InitShadowBuffers() {
    M_SHADOW_SCR_WIDTH = 4096 * 2;
    M_SHADOW_SCR_HEIGHT = 4096;

    glGenFramebuffers(1, &buffers.shadowMapFBO);
    glGenTextures(1, &buffers.shadowMapTexture);

    glBindTexture(GL_TEXTURE_2D, buffers.shadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, M_SHADOW_SCR_WIDTH, M_SHADOW_SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, buffers.shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffers.shadowMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TSR_Simulation::InitShaders() {
    m_shaderHandler.addShaders("standard", "Shaders/VertexShader.glsl", "Shaders/PixelShaderBlinnPhong.glsl");
    m_shaderHandler.addShaders("textured", "Shaders/VertexShader.glsl", "Shaders/PixelShaderTextures.glsl");
    // m_shaderHandler.addShaders("noShading", "Shaders/VertexShader.glsl", "Shaders/PixelShaderLights.glsl");
    m_shaderHandler.addShaders("water", "Shaders/VertexShader.glsl", "Shaders/PixelShaderWater.glsl");
    m_shaderHandler.useShader("textured");

    InitBuffers();
    m_shaderHandler.linkShaderUniformBlock("textured", "MaterialBlock", 0);
    m_shaderHandler.linkShaderUniformBlock("textured", "LightBlock", 1);
    m_shaderHandler.linkShaderUniformBlock("standard", "MaterialBlock", 0);
    m_shaderHandler.linkShaderUniformBlock("standard", "LightBlock", 1);
    m_shaderHandler.linkShaderUniformBlock("water", "MaterialBlock", 0);
    m_shaderHandler.linkShaderUniformBlock("water", "LightBlock", 1);

    m_shaderHandler.addShaders("picking", "Shaders/VertexShaderPickingOutline.glsl", "Shaders/PixelShaderPicking.glsl");
    m_shaderHandler.addShaders("outline", "Shaders/VertexShaderPickingOutline.glsl", "Shaders/PixelShaderPickedOutline.glsl");
    m_shaderHandler.addShaders("cubemap", "Shaders/VertexShaderCubemap.glsl", "Shaders/PixelShaderCubemap.glsl");

    m_shaderHandler.addShaders("shadow", "Shaders/VertexShaderShadow.glsl", "Shaders/PixelShaderShadow.glsl");
}

void TSR_Simulation::Update() {
    InputUpdate();
    ClutterUpdate();
    WaterUpdate();
    if (m_timer.getCounter3() > m_terrainGenerationSpeedSigns) {
        TerrainGenerationSigns();
        m_timer.resetCounter3();
    }

    if (m_timer.getCounter4() > m_terrainGenerationSpeedTrees) {
        TerrainGenerationTrees();
        m_timer.resetCounter4();
    }

    if (m_timer.getCounter5() > m_oppCerGenerationSpeed) {
        TerrainGenerationCar();
        m_timer.resetCounter5();
    }
}

void TSR_Simulation::WaterUpdate() {
    if (m_timer.getCounter2() > 0.25f) {

        int i = 4 + rand() % (((m_water.RowCount() - 5) - 4) + 1);
        int j = 4 + rand() % (((m_water.ColumnCount() - 5) - 4) + 1);;

        float r = 0.2f + ((float)(rand()) / (float)RAND_MAX) * (0.5f - 0.2f);

        if (i > 50 && i < 90) r = std::min(r, 0.1f);
        else r = std::min(r, 4.f);

        m_water.Disturb(i, j, r);

        m_timer.resetCounter2();
    }

    m_water.Update(m_timer.getDeltaTime());

    std::vector<Vertex> vers(m_water.VertexCount());
    for (int i = 0; i < m_water.VertexCount(); ++i) {
        Vertex v;

        v.position = m_water.Position(i);
        v.normal = m_water.Normal(i);

        vers[i] = v;
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffers.waterObject.geometry.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vers.size() * sizeof(Vertex), vers.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}


void TSR_Simulation::InputUpdate() {
    if (!m_isFirstPersonView) {
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
            m_cameraHandlerOuter.moveFront(m_timer.getDeltaTime());
        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
            m_cameraHandlerOuter.moveBack(m_timer.getDeltaTime());
        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
            m_cameraHandlerOuter.moveLeft(m_timer.getDeltaTime());
        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
            m_cameraHandlerOuter.moveRight(m_timer.getDeltaTime());
    }

    if (m_isCameraFree) {
        if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
            m_cameraHandlerOuter.lookLeft(m_timer.getDeltaTime());
        if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            m_cameraHandlerOuter.lookRight(m_timer.getDeltaTime());
        if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
            m_cameraHandlerOuter.lookUp(m_timer.getDeltaTime());
        if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
            m_cameraHandlerOuter.lookDown(m_timer.getDeltaTime());
    } else {
        if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            double mouseX, mouseY;
            glfwGetCursorPos(m_window, &mouseX, &mouseY);

            if (m_firstMouse) {
                m_lastMouseX = mouseX;
                m_lastMouseY = mouseY;
                m_firstMouse = false;
            }

            double deltaX = mouseX - m_lastMouseX;
            double deltaY = m_lastMouseY - mouseY;

            m_cameraHandlerOuter.lookAround(deltaX, deltaY);

            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;
        }
        else {
            m_firstMouse = true;
        }
    }

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
}

void TSR_Simulation::ClutterUpdate() {

    for (auto& it : m_objectHandler.getObjectsVectorType(ObjectType::OPP_CAR)) {
        std::vector<uint32_t> deleteElements;
        for (int i = 0; i < it->worldData.size(); i++) {
            if (it->worldData.at(i).Position.x < -30.f) {
                deleteElements.push_back(i);
            } else {
                it->worldData.at(i).Position.x -= m_oppCarSpeed * m_carSpeed * m_timer.getDeltaTime();
            }
        }

        for (auto it2 = deleteElements.rbegin(); it2 != deleteElements.rend(); ++it2) {
            if (*it2 < it->worldData.size()) {
                if (it == m_pickedRenderObject && *it2 < m_pickedObjectIndex) {
                    m_pickedObjectIndex--;
                }
                it->worldData.erase(it->worldData.begin() + *it2);
            }
        }
    }

    for (auto& it : m_objectHandler.getObjectsVectorType(ObjectType::CLUTTER)) {
        std::vector<uint32_t> deleteElements;
        for (int i = 0; i < it->worldData.size(); i++) {
            if (it->worldData.at(i).Position.x < -30.f) {
                deleteElements.push_back(i);
            } else {
                it->worldData.at(i).Position.x -= m_carSpeed * m_timer.getDeltaTime();
            }
        }

        for (auto it2 = deleteElements.rbegin(); it2 != deleteElements.rend(); ++it2) {
            if (*it2 < it->worldData.size()) {
                if (it == m_pickedRenderObject && *it2 < m_pickedObjectIndex) {
                    m_pickedObjectIndex--;
                }
                it->worldData.erase(it->worldData.begin() + *it2);
            }
        }
    }

    if ((m_carSpeed != m_carSpeedPrev) || m_passCounter >= 30) {
        m_passCounter = 0;
        std::shared_ptr<RenderObject> obj = m_objectHandler.getObject("road");
        obj->worldData.clear();
        WorldData wd;
        wd.Picked = false;
        wd.Scale = glm::vec3(4.f, 4.f, 8.f);
        wd.Position = glm::vec3(-30.f, 0.f, 0.f);
        wd.Rotation = glm::vec3(0.f, 0.f, 0.f);

        for (int i = 0; i < 30; i++) {
            m_objectHandler.addObjectInstance("road", wd);
            wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, 0.f);
        }

        obj = m_objectHandler.getObject("grass");
        obj->worldData.clear();

        wd.Scale = glm::vec3(4.f, 4.f, 4.f);

        for (int i = 0; i < 29; i++) {
            wd.Position = glm::vec3(-30.f + i * 3.99f, 0.0f, 0.f);

            wd.Position.z = 2.5f;
            m_objectHandler.addObjectInstance("grass", wd);
            wd.Position.z = 6.5f;
            m_objectHandler.addObjectInstance("grass", wd);
            wd.Position.z = 10.5f;
            m_objectHandler.addObjectInstance("grass", wd);

            wd.Position.z = -2.5f;
            m_objectHandler.addObjectInstance("grass", wd);
            wd.Position.z = -6.5f;
            m_objectHandler.addObjectInstance("grass", wd);
            wd.Position.z = -10.5f;
            m_objectHandler.addObjectInstance("grass", wd);
        }
    } else {
        bool pass = false;
        for (auto& it : m_objectHandler.getObjectsVectorType(ObjectType::ROAD)) {
            for (int i = 0; i < it->worldData.size(); i++) {
                if (it->worldData.at(i).Position.x < -30.f) {
                    it->worldData.at(i).Position.x = 85.692f;
                    pass = true;
                }
                else {
                    it->worldData.at(i).Position.x -= m_carSpeed * m_timer.getDeltaTime();
                }
            }
        }

        if (pass) {
            m_passCounter++;
        }
    }

    m_carSpeedPrev = m_carSpeed;
}

void TSR_Simulation::TerrainGenerationSigns() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, 1600);

    WorldData wd;

    uint32_t sig = distr(gen);

    std::uniform_int_distribution<> distrDR(0, 1);
    if (distrDR(gen) == 0) {
        wd.Position = glm::vec3(85.71f, 0.3f, 1.f);
        wd.Scale = glm::vec3(1.f, 1.f, 1.f);
        wd.Rotation = glm::vec3(0.f, 265.f, 0.f);
    }
    else {
        wd.Position = glm::vec3(85.71f, 0.3f, -1.f);
        wd.Scale = glm::vec3(1.f, 1.f, 1.f);
        wd.Rotation = glm::vec3(0.f, 85.f, 0.f);
    }

    if (sig <= 50 - 50 + m_generationBiases[0]) {
        m_objectHandler.addObjectInstance("20", wd);
    }
    else if (sig > 50 && sig <= 100 - 50 + m_generationBiases[1]) {
        m_objectHandler.addObjectInstance("60", wd);
    }
    else if (sig > 100 && sig <= 150 - 50 + m_generationBiases[2]) {
        m_objectHandler.addObjectInstance("odvzemPrednosti", wd);
    }
    else if (sig > 150 && sig <= 200 - 50 + m_generationBiases[3]) {
        m_objectHandler.addObjectInstance("stop", wd);
    }
    else if (sig > 250 && sig <= 300 - 50 + m_generationBiases[4]) {
        m_objectHandler.addObjectInstance("130", wd);
    }
    else if (sig > 350 && sig <= 400 - 50 + m_generationBiases[5]) {
        m_objectHandler.addObjectInstance("20-", wd);
    }
    else if (sig > 450 && sig <= 500 - 50 + m_generationBiases[6]) {
        m_objectHandler.addObjectInstance("30", wd);
    }
    else if (sig > 500 && sig <= 550 - 50 + m_generationBiases[7]) {
        m_objectHandler.addObjectInstance("30-", wd);
    }
    else if (sig > 550 && sig <= 600 - 50 + m_generationBiases[8]) {
        m_objectHandler.addObjectInstance("40", wd);
    }
    else if (sig > 600 && sig <= 650 - 50 + m_generationBiases[9]) {
        m_objectHandler.addObjectInstance("40-", wd);
    }
    else if (sig > 650 && sig <= 700 - 50 + m_generationBiases[10]) {
        m_objectHandler.addObjectInstance("50", wd);
    }
    else if (sig > 700 && sig <= 750 - 50 + m_generationBiases[11]) {
        m_objectHandler.addObjectInstance("50-", wd);
    }
    else if (sig > 750 && sig <= 800 - 50 + m_generationBiases[12]) {
        m_objectHandler.addObjectInstance("60-", wd);
    }
    else if (sig > 800 && sig <= 850 - 50 + m_generationBiases[13]) {
        m_objectHandler.addObjectInstance("70", wd);
    }
    else if (sig > 850 && sig <= 900 - 50 + m_generationBiases[14]) {
        m_objectHandler.addObjectInstance("70-", wd);
    }
    else if (sig > 900 && sig <= 950 - 50 + m_generationBiases[15]) {
        m_objectHandler.addObjectInstance("80", wd);
    }
    else if (sig > 950 && sig <= 1000 - 50 + m_generationBiases[16]) {
        m_objectHandler.addObjectInstance("80-", wd);
    }
    else if (sig > 1000 && sig <= 1050 - 50 + m_generationBiases[17]) {
        m_objectHandler.addObjectInstance("90-", wd);
    }
    else if (sig > 1050 && sig <= 1100 - 50 + m_generationBiases[18]) {
        m_objectHandler.addObjectInstance("100", wd);
    }
    else if (sig > 1100 && sig <= 1150 - 50 + m_generationBiases[19]) {
        m_objectHandler.addObjectInstance("100-", wd);
    }
    else if (sig > 1150 && sig <= 1200 - 50 + m_generationBiases[20]) {
        m_objectHandler.addObjectInstance("110", wd);
    }
    else if (sig > 1200 && sig <= 1250 - 50 + m_generationBiases[21]) {
        m_objectHandler.addObjectInstance("110-", wd);
    }
    else if (sig > 1250 && sig <= 1300 - 50 + m_generationBiases[22]) {
        /*m_objectHandler.addObjectInstance("120", wd);*/
    }
    else if (sig > 1300 && sig <= 1350 - 50 + m_generationBiases[23]) {
        /*m_objectHandler.addObjectInstance("120-", wd);*/
    }
    else if (sig > 1350 && sig <= 1400 - 50 + m_generationBiases[24]) {
        /*m_objectHandler.addObjectInstance("130-", wd);*/
    }
    else if (sig > 1450 && sig <= 1500 - 50 + m_generationBiases[25]) {
        m_objectHandler.addObjectInstance("konecvsehomejitev", wd);
    }
    else if (sig > 1500 && sig <= 1550 - 50 + m_generationBiases[17]) {
        m_objectHandler.addObjectInstance("90", wd);
    }
}

void TSR_Simulation::TerrainGenerationTrees() {
    std::random_device rd;
    std::mt19937 gen(rd());

    WorldData dat;
    std::uniform_real_distribution<> distrY(-10, 10);
    std::uniform_real_distribution<> distrO(2, 10);
    std::uniform_real_distribution<> distrU(-10, -2);
    std::uniform_real_distribution<> distrS(2, 4);
    std::uniform_real_distribution<> distrR(0, 360);
    dat.Position = glm::vec3(85.71f, 1.f, distrY(gen));
    dat.Rotation = glm::vec3(0.f, distrR(gen), 0.f);

    if (dat.Position.z > -2 && dat.Position.z < 2) {
        if (dat.Position.z < 0) {
            dat.Position.z = distrU(gen);
        }
        else {
            dat.Position.z = distrO(gen);
        }
    }

    dat.Scale = glm::vec3(distrS(gen), distrS(gen), distrS(gen));
    m_objectHandler.addObjectInstance("tree", dat);
}

void TSR_Simulation::TerrainGenerationCar() {
    std::random_device rd;
    std::mt19937 gen(rd());

    WorldData dat;
    std::uniform_int_distribution<> distr(0, 100);
    if (distr(gen) < m_oppCarBias) {
        dat.Position = glm::vec3(85.71f, 0.15f, -0.25f);
        dat.Rotation = glm::vec3(0.f, 180.f, 0.f);
        dat.Scale = glm::vec3(0.7f, 0.7f, 0.7f);
        m_objectHandler.addObjectInstance("carOpp", dat);
    }
}


void TSR_Simulation::LoadTrafficSignTexture(const std::string& sign) {
    if (m_currentSignTexture != 0) {
        glDeleteTextures(1, &m_currentSignTexture);
    }
    std::string path = "assets/" + sign + ".png";  // Adjust path as needed
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return;
    }

    glGenTextures(1, &m_currentSignTexture);
    glBindTexture(GL_TEXTURE_2D, m_currentSignTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
}



void TSR_Simulation::Draw() {
    //Here we'll call all draw pass functions such as the draw pass, picking pass, shadow pass, outline pass...

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    static std::chrono::steady_clock::time_point m_lastDetectionTime;
    static bool m_signVisible = false;


    ImGui::Begin("Settings");
    if (ImGui::RadioButton("First Person View", m_isFirstPersonView)) {
        m_isFirstPersonView = true;
        CameraConfig conf;
        conf.Position = glm::vec3(-0.1f, 0.2f, 0.25f);
        conf.Front = glm::vec3(0.0f, 0.0f, 1.0f);
        conf.Up = glm::vec3(0.0f, 1.0f, 0.0f);
        conf.projection = glm::perspective(glm::radians(45.0f), (float)M_SCR_WIDTH / (float)M_SCR_HEIGHT, 0.01f, 100.f);
        conf.sensitivity = 80.f;
        conf.pitch = 0.f;
        conf.yaw = 0.f;
        conf.speed = 10.f;
        m_cameraHandlerOuter = CameraHandler(conf, glm::vec3(0.1f, 0.15f, 0.3f), false);

        conf.speed = 0.f;
        conf.sensitivity = 0.f;
        conf.Position = glm::vec3(-3.0f, 1.0f, 0.0f);
        m_cameraHandlerInner = CameraHandler(conf);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Third Person View", !m_isFirstPersonView)) {
        m_isFirstPersonView = false;

        CameraConfig conf;
        conf.Position = glm::vec3(-4.0f, 3.0f, 3.0f);
        conf.Front = glm::vec3(0.0f, 0.0f, 1.0f);
        conf.Up = glm::vec3(0.0f, 1.0f, 0.0f);
        conf.projection = glm::perspective(glm::radians(45.0f), (float)M_SCR_WIDTH / (float)M_SCR_HEIGHT, 0.01f, 100.f);
        conf.sensitivity = 80.f;
        conf.pitch = 0.f;
        conf.yaw = 0.f;
        conf.speed = 10.f;
        m_cameraHandlerOuter = CameraHandler(conf, glm::vec3(0.f, 0.2f, 0.f), false);

        conf.speed = 0.f;
        conf.sensitivity = 0.f;
        conf.Position = glm::vec3(-0.1f, 0.2f, 0.25f);
        m_cameraHandlerInner = CameraHandler(conf);
    }
    ImGui::End();

    if (m_showPerformance) {
        ImGui::Begin("Performance", &m_showPerformance);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            m_timer.getDeltaTime(), ImGui::GetIO().Framerate);

        static float fpsHistory[100] = {};
        static int fpsOffset = 0;
        fpsHistory[fpsOffset] = ImGui::GetIO().Framerate;
        fpsOffset = (fpsOffset + 1) % IM_ARRAYSIZE(fpsHistory);

        ImGui::PlotLines("FPS", fpsHistory, IM_ARRAYSIZE(fpsHistory), fpsOffset,
            "Frame Time", 0.0f, 200.0f, ImVec2(0, 80.0f));

        ImGui::End();
    }



    // Controls Guide Popup
    if (m_showControlsGuide) {
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Controls Guide", &m_showControlsGuide)) {
            ImGui::Text("Movement Controls:");
            ImGui::BulletText("W/S - Move forward/backward");
            ImGui::BulletText("A/D - Move left/right");
            ImGui::BulletText("Arrow Keys - Look around");
            ImGui::BulletText("ESC - Exit application");
            // Add more controls info
        }
        ImGui::End();
    }



    ImGui::Begin("Controlls");

    ImGui::Image(buffers.secondViewTexture, ImVec2(M_CAR_SCR_WIDTH / 4, M_CAR_SCR_HEIGHT / 4), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

    if (ImGui::CollapsingHeader("AI sign recognition")) {
        ImGui::BeginChild("Traffic Sign Detection", ImVec2(250.f, 250.f));
        if (g_newDataReceived) {
            if (g_lastReceivedResult == -1) {
                m_showTrafficSign = false;
                m_currentSign = "unknown";
                m_signVisible = false;
            } else {
                for (const auto& [sign, index] : traffic_signs) {
                    if (index == g_lastReceivedResult) {
                        if (m_currentSign != sign) {
                            m_currentSign = sign;
                            LoadTrafficSignTexture(sign);
                            m_lastDetectionTime = std::chrono::steady_clock::now();
                            m_signVisible = true;
                        }
                        break;
                    }
                }
            }
            g_newDataReceived = false;
        }

        auto now = std::chrono::steady_clock::now();
        float elapsedSeconds = std::chrono::duration<float>(now - m_lastDetectionTime).count();
        float aspectRatio = 1.0f;
        float displaySize = 200.0f;
        //ImGui::Image((ImTextureID)(intptr_t)m_currentSignTexture, ImVec2(displaySize, displaySize * aspectRatio));

        if (elapsedSeconds <= 3.0f && m_currentSign != "unknown") {

            ImGui::Text("Detected Sign: %s", m_currentSign.c_str());
            ImGui::Image((ImTextureID)(intptr_t)m_currentSignTexture, ImVec2(displaySize, displaySize * aspectRatio));
        } else {
            ImGui::Text("No sign detected");
        }
        ImGui::EndChild();
    }

    ImGui::Checkbox("Capture & Save POV images", &m_imageCapture);
    ImGui::Checkbox("Send POV images", &m_imageSend);
    ImGui::Checkbox("Enable picking", &m_enablePicking);
    ImGui::SliderFloat("Capture interval", &m_imageCaptureInterval, 0.f, 10.f, "%.3f s");
    ImGui::SliderInt("Car speed", &m_carSpeed, 0.001f, 10.f, "%d s");
    ImGui::SliderFloat("Opp car speed", &m_oppCarSpeed, 0.001f, 10.f, "%.3f s");
    ImGui::SliderFloat("Terrain generation speed(Signs)", &m_terrainGenerationSpeedSigns, 0.001f, 10.f, "%.3f s");
    ImGui::SliderFloat("Terrain generation speed(Trees)", &m_terrainGenerationSpeedTrees, 0.001f, 10.f, "%.3f s");
    ImGui::SliderFloat("Terrain generation speed(Cars)", &m_oppCerGenerationSpeed, 0.001f, 10.f, "%.3f s");
    ImGui::SliderInt("Opp car bias", &m_oppCarBias, 0, 100, "%d s");

    if (ImGui::CollapsingHeader("Bias controll")) {
        propabilityMenu();
    }

    if (ImGui::Button("Locked camera")) {
        m_isCameraFree = false;
        m_cameraHandlerOuter.setType(1.f, 1.f, true);
    } else if (ImGui::Button("Free camera")) {
        m_isCameraFree = true;
        m_cameraHandlerOuter.setType(10.f, 80.f, false);
    }

    m_menuHover = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) || m_menuHover;

    ImGui::End();

    ShadowMapDrawPass();

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (!m_menuHover && m_enablePicking) {
        PickingDrawPass();
    }

    ObjectDrawPass(CameraType::OUTSIDE_CAMERA);
    WaterDraw(CameraType::OUTSIDE_CAMERA);
    RenderedWaterDrawPass();
    CubemapDrawPass(CameraType::OUTSIDE_CAMERA);

    m_menuHover = false;

    if (m_pickedObjectIndex != -1) {
        OutlineDrawPass();
    }

    if (m_timer.getCounter1() >= m_imageCaptureInterval) {
        glBindFramebuffer(GL_FRAMEBUFFER, buffers.secondViewFBO);
        glViewport(0, 0, M_CAR_SCR_WIDTH, M_CAR_SCR_HEIGHT);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ObjectDrawPass(CameraType::INSIDE_CAMERA);
        //WaterDraw(CameraType::INSIDE_CAMERA);
        CubemapDrawPass(CameraType::INSIDE_CAMERA);

        std::shared_ptr<std::vector<unsigned char>> pixels = std::make_shared<std::vector<unsigned char>>((M_CAR_SCR_WIDTH * M_CAR_SCR_HEIGHT * 3));
        glReadPixels(0, 0, M_CAR_SCR_WIDTH, M_CAR_SCR_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels->data());

        if (m_imageCapture) {
            std::thread t(std::bind(&TSR_Simulation::saveFboToImage, this, pixels));
            t.detach();
        }

        if (m_imageSend) {
            std::thread t(std::bind(&TSR_Simulation::sendScreenCapture, this, pixels));
            t.detach();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT);

        m_timer.resetCounter1();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void TSR_Simulation::WaterDraw(CameraType type) {

    m_shaderHandler.setInt("standard", "numOfLights", m_lights.size());
    m_shaderHandler.setVec3("standard", "ambientColor", m_ambientColor);


    if (type == CameraType::OUTSIDE_CAMERA) {
        m_shaderHandler.setMat4x4("standard", "projectionView", m_cameraHandlerOuter.getProjection() * m_cameraHandlerOuter.getView());
        m_shaderHandler.setVec3("standard", "cameraPos", m_cameraHandlerOuter.getCameraPos());
    }
    else {
        m_shaderHandler.setMat4x4("standard", "projectionView", m_cameraHandlerInner.getProjection() * m_cameraHandlerInner.getView());
        m_shaderHandler.setVec3("standard", "cameraPos", m_cameraHandlerInner.getCameraPos());
    }

    glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    m_shaderHandler.useShader("standard");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, buffers.shadowMapTexture);

    m_shaderHandler.setInt("standard", "shadowMap", 0);

    glBindVertexArray(buffers.waterObject.geometry.VAO);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilMask(0xFF);
    m_shaderHandler.setMat4x4("standard", "world", buffers.waterObject.worldData.at(0).getWorldTransform());

    glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &buffers.waterObject.material.at(0));

    glDrawElements(GL_TRIANGLES, buffers.indexCount, GL_UNSIGNED_INT, 0);

}

void TSR_Simulation::PickingDrawPass() {
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glDisable(GL_BLEND);
        m_shaderHandler.setMat4x4("picking", "projectionView", m_cameraHandlerOuter.getProjection() * m_cameraHandlerOuter.getView());

        double mouseX, mouseY;
        glfwGetCursorPos(m_window, &mouseX, &mouseY);

        int readX = static_cast<int>(mouseX);
        int readY = M_SCR_HEIGHT - static_cast<int>(mouseY) - 1;

        glBindFramebuffer(GL_FRAMEBUFFER, buffers.pickingFBO);
        glViewport(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //This goes into a loop for each object
        for (const auto& obj : m_objectHandler.getObjectsVector()) {
            glBindVertexArray(obj->geometry.VAO);
            for (int j = 0; j < obj->geometry.startIndexies.size(); j++) {

                //This goes into a sub-loop for each instance of an object
                //There is currently only one object with one instance
                for (int i = 0; i < obj->worldData.size(); i++) {
                    m_shaderHandler.setMat4x4("picking", "world", obj->worldData.at(i).getWorldTransform());

                    GLubyte r = ((obj->objectID + 1) & 0xFF);
                    GLubyte g = ((i + 1) & 0xFF);
                    m_shaderHandler.setVec3("picking", "idColor", glm::vec3(r / 255.f, g / 255.f, 0));

                    glDrawElements(GL_TRIANGLES, obj->geometry.numOfIndecies[j], GL_UNSIGNED_INT, (void*)(obj->geometry.startIndexies[j] * sizeof(unsigned int)));
                }

            }
            glBindVertexArray(0);
        }

        GLubyte pixelColor[3];
        glReadPixels(readX, readY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixelColor);

        if (!m_menuHover) {
            if ((int)pixelColor[0] != 0 && (int)pixelColor[1] != 0) {
                if (m_pickedObjectIndex != -1) {
                    m_pickedRenderObject->worldData.at(m_pickedObjectIndex).Picked = false;
                }
                m_pickedRenderObject = m_objectHandler.getObjectsVector()[(int)pixelColor[0] - 1];
                m_pickedObjectIndex = (int)pixelColor[1] - 1;
                m_pickedRenderObject->worldData.at(m_pickedObjectIndex).Picked = true;
            }
            else {
                if (m_pickedObjectIndex != -1) {
                    m_pickedRenderObject->worldData.at(m_pickedObjectIndex).Picked = false;
                    m_pickedRenderObject = nullptr;
                }
                m_pickedObjectIndex = -1;
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT);
        glEnable(GL_BLEND);
    }
}

void TSR_Simulation::ObjectDrawPass(CameraType type) {
    m_shaderHandler.setInt("standard", "numOfLights", m_lights.size());
    m_shaderHandler.setVec3("standard", "ambientColor", m_ambientColor);
    m_shaderHandler.setInt("textured", "numOfLights", m_lights.size());
    m_shaderHandler.setVec3("textured", "ambientColor", m_ambientColor);
    m_shaderHandler.setMat4x4("standard", "lightSpaceMatrix", m_cameraHandlerShadow.getProjection() * m_cameraHandlerShadow.getView());
    m_shaderHandler.setMat4x4("textured", "lightSpaceMatrix", m_cameraHandlerShadow.getProjection() * m_cameraHandlerShadow.getView());
    m_shaderHandler.setInt("water", "numOfLights", m_lights.size());
    m_shaderHandler.setVec3("water", "ambientColor", m_ambientColor);

    if (type == CameraType::OUTSIDE_CAMERA) {
        m_shaderHandler.setMat4x4("textured", "projectionView", m_cameraHandlerOuter.getProjection() * m_cameraHandlerOuter.getView());
        m_shaderHandler.setVec3("textured", "cameraPos", m_cameraHandlerOuter.getCameraPos());
        m_shaderHandler.setMat4x4("standard", "projectionView", m_cameraHandlerOuter.getProjection() * m_cameraHandlerOuter.getView());
        m_shaderHandler.setVec3("standard", "cameraPos", m_cameraHandlerOuter.getCameraPos());
        m_shaderHandler.setMat4x4("water", "projectionView", m_cameraHandlerOuter.getProjection() * m_cameraHandlerOuter.getView());
        m_shaderHandler.setVec3("water", "cameraPos", m_cameraHandlerOuter.getCameraPos());
    }
    else {
        m_shaderHandler.setMat4x4("textured", "projectionView", m_cameraHandlerInner.getProjection() * m_cameraHandlerInner.getView());
        m_shaderHandler.setVec3("textured", "cameraPos", m_cameraHandlerInner.getCameraPos());
        m_shaderHandler.setMat4x4("standard", "projectionView", m_cameraHandlerInner.getProjection() * m_cameraHandlerInner.getView());
        m_shaderHandler.setVec3("standard", "cameraPos", m_cameraHandlerInner.getCameraPos());
        m_shaderHandler.setMat4x4("water", "projectionView", m_cameraHandlerInner.getProjection() * m_cameraHandlerInner.getView());
        m_shaderHandler.setVec3("water", "cameraPos", m_cameraHandlerInner.getCameraPos());
    }

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    for (auto& obj : m_objectHandler.getObjectsVector()) {
        /*if (type == CameraType::INSIDE_CAMERA && obj->Type == ObjectType::CAR) {
            continue;
        }*/

        if (obj->geometry.textures.size() != 0) {
            ObjectDrawPassTextured(obj);
        }
        else {
            ObjectDrawPassUntextured(obj);
        }
    }
}


void TSR_Simulation::ObjectDrawPassTextured(std::shared_ptr<RenderObject>& obj) {
    m_shaderHandler.useShader("textured");

    glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, buffers.shadowMapTexture);

    m_shaderHandler.setInt("textured", "texture_diffuse", 0);

    glBindVertexArray(obj->geometry.VAO);
    for (int j = 0; j < obj->geometry.numOfIndecies.size(); j++) {
        for (int i = 0; i < obj->worldData.size(); i++) {

            m_shaderHandler.setMat4x4("textured", "world", obj->worldData.at(i).getWorldTransform());

            if (obj->worldData.at(i).Picked) {
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
            }
            else {
                glStencilFunc(GL_ALWAYS, 0, 0xFF);
                glStencilMask(0xFF);
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj->geometry.textures[j].texture);

            m_shaderHandler.setInt("textured", "shadowMap", 1);

            glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &obj->material.at(j));

            glDrawElements(GL_TRIANGLES, obj->geometry.numOfIndecies[j], GL_UNSIGNED_INT, (void*)(obj->geometry.startIndexies[j] * sizeof(unsigned int)));
        }
    }
}

void TSR_Simulation::ObjectDrawPassUntextured(std::shared_ptr<RenderObject>& obj) {
    glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    m_shaderHandler.useShader("standard");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, buffers.shadowMapTexture);

    m_shaderHandler.setInt("textured", "shadowMap", 0);

    glBindVertexArray(obj->geometry.VAO);
    for (int j = 0; j < obj->geometry.numOfIndecies.size(); j++) {
        for (int i = 0; i < obj->worldData.size(); i++) {

            if (obj->worldData.at(i).Picked) {
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
            }
            else {
                glStencilFunc(GL_ALWAYS, 0, 0xFF);
                glStencilMask(0xFF);
            }

            m_shaderHandler.setMat4x4("standard", "world", obj->worldData.at(i).getWorldTransform());

            glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &obj->material.at(j));

            glDrawElements(GL_TRIANGLES, obj->geometry.numOfIndecies[j], GL_UNSIGNED_INT, (void*)(obj->geometry.startIndexies[j] * sizeof(unsigned int)));
        }
    }
}

void TSR_Simulation::OutlineDrawPass() {
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    m_shaderHandler.setMat4x4("outline", "projectionView", m_cameraHandlerOuter.getProjection() * m_cameraHandlerOuter.getView());

    glBindVertexArray(m_pickedRenderObject->geometry.VAO);
    if (m_pickedRenderObject->worldData.at(m_pickedObjectIndex).Picked) {
        for (int j = 0; j < m_pickedRenderObject->geometry.numOfIndecies.size(); j++) {
            m_shaderHandler.setMat4x4("outline", "world", m_pickedRenderObject->worldData.at(m_pickedObjectIndex).getPickedTransform());
            glDrawElements(GL_TRIANGLES, m_pickedRenderObject->geometry.numOfIndecies[j], GL_UNSIGNED_INT, (void*)(m_pickedRenderObject->geometry.startIndexies[j] * sizeof(unsigned int)));
        }
    }

    glEnable(GL_DEPTH_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glDisable(GL_STENCIL_TEST);
}

void TSR_Simulation::CubemapDrawPass(CameraType type) {
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    if (type == CameraType::OUTSIDE_CAMERA) {
        m_shaderHandler.setMat4x4("cubemap", "projectionView", m_cameraHandlerOuter.getProjection() * glm::mat4(glm::mat3(m_cameraHandlerOuter.getView())));
    }
    else {
        m_shaderHandler.setMat4x4("cubemap", "projectionView", m_cameraHandlerInner.getProjection() * glm::mat4(glm::mat3(m_cameraHandlerInner.getView())));
    }

    glBindVertexArray(buffers.cubemapVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, buffers.cubemapTexture.texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}

void TSR_Simulation::ShadowMapDrawPass() {
    glViewport(0, 0, M_SHADOW_SCR_WIDTH, M_SHADOW_SCR_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, buffers.shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    //glDisable(GL_MULTISAMPLE);

    m_shaderHandler.setMat4x4("shadow", "projectionView", m_cameraHandlerShadow.getProjection() * m_cameraHandlerShadow.getView());

    for (auto& obj : m_objectHandler.getObjectsVector()) {
        if (obj->Type == ObjectType::ROAD) {
            continue;
        }
        glBindVertexArray(obj->geometry.VAO);
        for (int j = 0; j < obj->geometry.numOfIndecies.size(); j++) {
            for (int i = 0; i < obj->worldData.size(); i++) {

                if (obj->worldData.at(i).Picked) {
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    glStencilMask(0xFF);
                }
                else {
                    glStencilFunc(GL_ALWAYS, 0, 0xFF);
                    glStencilMask(0xFF);
                }

                m_shaderHandler.setMat4x4("shadow", "world", obj->worldData.at(i).getWorldTransform());

                glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
                glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &obj->material.at(j));

                glDrawElements(GL_TRIANGLES, obj->geometry.numOfIndecies[j], GL_UNSIGNED_INT, (void*)(obj->geometry.startIndexies[j] * sizeof(unsigned int)));
            }
        }
    }

    /*glEnable(GL_MULTISAMPLE);*/

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT);
}

void TSR_Simulation::RenderedWaterDrawPass() {

    m_shaderHandler.useShader("water");

    m_shaderHandler.setInt("water", "numOfLights", m_lights.size());
    m_shaderHandler.setVec3("water", "ambientColor", m_ambientColor);
    m_shaderHandler.setMat4x4("water", "projectionView", m_cameraHandlerOuter.getProjection() * m_cameraHandlerOuter.getView());
    m_shaderHandler.setVec3("water", "cameraPos", m_cameraHandlerOuter.getCameraPos());

    float currentTime = static_cast<float>(glfwGetTime());
    m_shaderHandler.setFloat("water", "time", currentTime);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, buffers.water.geometry.textures[0].texture);


    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, buffers.water.geometry.textures[1].texture);
    m_shaderHandler.setInt("water", "texture_diffuse2", 2);

    m_shaderHandler.setInt("water", "texture_diffuse", 0);
    m_shaderHandler.setInt("water", "shadowMap", 1);

    glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, buffers.shadowMapTexture);

    glBindVertexArray(buffers.water.geometry.VAO);

    glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &buffers.water.material.at(0));
    for (int i = 0; i < buffers.water.worldData.size(); i++) {
        m_shaderHandler.setMat4x4("water", "world", buffers.water.worldData.at(i).getWorldTransform());

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    }
}

void TSR_Simulation::sendScreenCapture(std::shared_ptr<std::vector<unsigned char>> pixels) {
    try {
        std::string broker = "10.8.2.2:9092";
        std::string topic_name = "sim-apk-pictures";

        if (pixels->size() != M_CAR_SCR_WIDTH * M_CAR_SCR_HEIGHT * 3) {
            throw std::runtime_error("The size of the pixels array does not match the specified dimensions.");
        }

        cv::Mat input_image(M_CAR_SCR_HEIGHT, M_CAR_SCR_WIDTH, CV_8UC3, pixels->data());
        cv::cvtColor(input_image, input_image, cv::COLOR_BGR2RGB);
        cv::flip(input_image, input_image, 0);

        Producer producer(broker, topic_name);
        producer.produceMessage(input_image);
        producer.flush();

        std::cout << "JSON message sent to topic: " << topic_name << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

void TSR_Simulation::saveFboToImage(std::shared_ptr<std::vector<unsigned char>> pixels) {

    std::vector<unsigned char> flippedPixels(M_CAR_SCR_WIDTH * M_CAR_SCR_HEIGHT * 3);
    for (int y = 0; y < M_CAR_SCR_HEIGHT; y++) {
        std::memcpy(&flippedPixels[y * M_CAR_SCR_WIDTH * 3], &pixels->at((M_CAR_SCR_HEIGHT - 1 - y) * M_CAR_SCR_WIDTH * 3), M_CAR_SCR_WIDTH * 3);
    }

    std::string fileName = "TrainingImages/screenCapture" + std::to_string(m_capturedImageIndex) + ".jpg";

    stbi_write_jpg(fileName.c_str(), M_CAR_SCR_WIDTH, M_CAR_SCR_HEIGHT, 3, flippedPixels.data(), 90);

    m_capturedImageIndex++;
}

void TSR_Simulation::propabilityMenu() {
    ImGui::BeginChild("Propabilities", ImVec2(0, 300), true);
    m_menuHover = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) || m_menuHover;
    ImGui::SliderInt("20", &m_generationBiases[0], 0, 50, "%d s");
    ImGui::SliderInt("60", &m_generationBiases[1], 0, 50, "%d s");
    ImGui::SliderInt("odvzemPrednosti", &m_generationBiases[2], 0, 50, "%d s");
    ImGui::SliderInt("stop", &m_generationBiases[3], 0, 50, "%d s");
    ImGui::SliderInt("130", &m_generationBiases[4], 0, 50, "%d s");
    ImGui::SliderInt("20-", &m_generationBiases[5], 0, 50, "%d s");
    ImGui::SliderInt("30", &m_generationBiases[6], 0, 50, "%d s");
    ImGui::SliderInt("30-", &m_generationBiases[7], 0, 50, "%d s");
    ImGui::SliderInt("40", &m_generationBiases[8], 0, 50, "%d s");
    ImGui::SliderInt("40-", &m_generationBiases[9], 0, 50, "%d s");
    ImGui::SliderInt("50", &m_generationBiases[10], 0, 50, "%d s");
    ImGui::SliderInt("50-", &m_generationBiases[11], 0, 50, "%d s");
    ImGui::SliderInt("60-", &m_generationBiases[12], 0, 50, "%d s");
    ImGui::SliderInt("70", &m_generationBiases[13], 0, 50, "%d s");
    ImGui::SliderInt("70-", &m_generationBiases[14], 0, 50, "%d s");
    ImGui::SliderInt("80", &m_generationBiases[15], 0, 50, "%d s");
    ImGui::SliderInt("80-", &m_generationBiases[16], 0, 50, "%d s");
    ImGui::SliderInt("90", &m_generationBiases[17], 0, 50, "%d s");
    ImGui::SliderInt("90-", &m_generationBiases[18], 0, 50, "%d s");
    ImGui::SliderInt("100", &m_generationBiases[19], 0, 50, "%d s");
    ImGui::SliderInt("100-", &m_generationBiases[20], 0, 50, "%d s");
    ImGui::SliderInt("110", &m_generationBiases[21], 0, 50, "%d s");
    ImGui::SliderInt("110-", &m_generationBiases[22], 0, 50, "%d s");
    ImGui::SliderInt("120", &m_generationBiases[23], 0, 50, "%d s");
    ImGui::SliderInt("120-", &m_generationBiases[24], 0, 50, "%d s");
    ImGui::SliderInt("130-", &m_generationBiases[25], 0, 50, "%d s");
    ImGui::SliderInt("konecvsehomejitev", &m_generationBiases[26], 0, 50, "%d s");
    ImGui::EndChild();
}

TSR_Simulation::TSR_Simulation() {
    Init();
}

TSR_Simulation::~TSR_Simulation() {
    glDeleteBuffers(1, &buffers.materialUBO);
    glDeleteBuffers(1, &buffers.lightsUBO);
    glDeleteTextures(1, &buffers.pickingTexture);
    glDeleteTextures(1, &buffers.secondViewTexture);
    glDeleteTextures(1, &buffers.cubemapTexture.texture);
    glDeleteTextures(1, &buffers.shadowMapTexture);
    glDeleteFramebuffers(1, &buffers.pickingFBO);
    glDeleteFramebuffers(1, &buffers.shadowMapFBO);
    glDeleteFramebuffers(1, &buffers.secondViewFBO);
    glDeleteVertexArrays(1, &buffers.cubemapVAO);
    glDeleteBuffers(1, &buffers.cubemapVBO);
    glDeleteVertexArrays(1, &buffers.waterObject.geometry.VAO);
    glDeleteBuffers(1, &buffers.waterObject.geometry.VBO);
    glDeleteBuffers(1, &buffers.waterObject.geometry.EBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

int TSR_Simulation::Run() {

    while (!glfwWindowShouldClose(m_window)) {
        m_timer.startTime();

        Update();

        Draw();

        m_timer.update();
    }

    glfwTerminate();

    return 0;
}