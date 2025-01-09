#include "TSR_Simulation.h"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void TSR_Simulation::Init() {
    InitGLFW();
    InitCamera();
    InitOpenGL();
    InitRenderObjects();
    InitCubemap();
    InitLights();
    InitShaders();
}

void TSR_Simulation::InitCamera() {
    CameraConfig conf;
    conf.Position = glm::vec3(0.0f, 0.0f, 3.0f);
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
    conf.Position = glm::vec3(0.f, 0.2f, 0.3f);
    m_cameraHandlerInner = CameraHandler(conf);
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

    m_window = glfwCreateWindow(M_SCR_WIDTH, M_SCR_HEIGHT, "TSR Simulation", /*NULL*/glfwGetPrimaryMonitor(), NULL);

    if (m_window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
}

void TSR_Simulation::InitOpenGL() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0x00);
}

void TSR_Simulation::InitRenderObjects() {
    /*m_objectHandler.addGeometry("car", "Models/model_avta.obj");
    m_objectHandler.addGeometry("street", "Models/street.obj");
    m_objectHandler.addMaterial("carMat", glm::vec4(0.f, 0.4f, 0.8f, 1.f), glm::vec3(0.6f, 0.6f, 0.6f), 0.5f);
    m_objectHandler.addMaterial("streetMat", glm::vec4(0.5f, 0.5f, 0.5f, 1.f), glm::vec3(0.5f, 0.5f, 0.5f), 0.8f);
    m_objectHandler.addTexture("street", "Textures/street.jpg");
    m_objectHandler.bindObject("car", "car", "", "carMat", ObjectType::CAR);
    m_objectHandler.bindObject("street", "street", "street", "streetMat");

    m_objectHandler.addGeometry("grassPlane", "Models/grass2.obj");
    m_objectHandler.addMaterial("grassMat", glm::vec4(0.5f, 0.5f, 0.5f, 1.f), glm::vec3(0.5f, 0.5f, 0.5f), 0.f);
    m_objectHandler.addTexture("grassTex", "Textures/grass.jpg");
    m_objectHandler.bindObject("grass", "grassPlane", "grassTex", "grassMat");*/
   /* m_objectHandler.loadOBJ("car", "Models/road.obj", ObjectType::CAR);

    WorldData wd;
    wd.Picked = false;
    wd.Position = glm::vec3(0.f, 0.2f, 0.f);
    wd.Scale = glm::vec3(1.f, 1.f, 0.8f);

    m_objectHandler.addObjectInstance("car", wd);*/
    WorldData wd;
    wd.Picked = false;
    wd.Scale = glm::vec3(4.f, 4.f, 8.f);
    wd.Position = glm::vec3(-30.f, 0.f, 0.f);
    wd.Rotation = glm::vec3(0.f, 0.f, 0.f);

    m_objectHandler.loadOBJ("road", "Models/road2.obj");

    for (int i = 0; i < 30; i++) {
        m_objectHandler.addObjectInstance("road", wd);
        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, 0.f);
    }

    m_objectHandler.loadOBJ("grass", "Models/bestgrass.obj");

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

    m_objectHandler.loadOBJ("20", "Models/20.obj");
    wd.Position = glm::vec3(0.f, 0.2f, 1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("20", wd);

    m_objectHandler.loadOBJ("60", "Models/60.obj");
    wd.Position = glm::vec3(4.f, 0.2f, 1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("60", wd);

    m_objectHandler.loadOBJ("odvzemPrednosti", "Models/odvzemPrednosti.obj");
    wd.Position = glm::vec3(20.f, 0.2f, 1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("odvzemPrednosti", wd);

    m_objectHandler.loadOBJ("stop", "Models/stop.obj");
    wd.Position = glm::vec3(40.f, 0.2f, -1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("stop", wd);

    m_objectHandler.loadOBJ("130", "Models/130.obj");
    wd.Position = glm::vec3(-20.f, 0.2f, -1.f);
    wd.Scale = glm::vec3(1.f, 1.f, 1.f);
    wd.Rotation = glm::vec3(0.f, 270.f, 0.f);
    m_objectHandler.addObjectInstance("130", wd);

    m_objectHandler.loadOBJ("tree", "Models/tree2.obj");
    wd.Scale = glm::vec3(2.f, 2.f, 2.f);
    wd.Position = glm::vec3(10.f, 1.f, 3.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(2.f, 3.f, 2.f);
    wd.Position = glm::vec3(20.f, 1.f, -4.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(2.f, 4.f, 2.f);
    wd.Position = glm::vec3(-20.f, 1.f, 5.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(3.f, 3.f, 3.f);
    wd.Position = glm::vec3(50.f, 1.f, 6.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(3.f, 2.f, 3.f);
    wd.Position = glm::vec3(70.f, 1.f, -5.f);
    m_objectHandler.addObjectInstance("tree", wd);

    m_objectHandler.loadOBJ("car", "Models/car.obj", ObjectType::CAR);
    wd.Scale = glm::vec3(0.7f, 0.7f, 0.7f);
    wd.Position = glm::vec3(0.f, 0.15f, 0.3f);
    wd.Rotation.y = 0.f;
    m_objectHandler.addObjectInstance("car", wd);



    /*wd.Scale = glm::vec3(4.f, 1.f, 4.f);
    wd.Position = glm::vec3(0.f, 0.f, 2.5f);
    wd.Rotation = glm::vec3(0.f, 0.f, 0.f);

    for (int i = 0; i < 30; i++) {
        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, 2.5f);
        m_objectHandler.addObjectInstance("grass", wd);
        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, 6.4f);
        m_objectHandler.addObjectInstance("grass", wd);
        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, 10.3f);
        m_objectHandler.addObjectInstance("grass", wd);

        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, -2.5f);
        m_objectHandler.addObjectInstance("grass", wd);
        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, -6.4f);
        m_objectHandler.addObjectInstance("grass", wd);
        wd.Position = glm::vec3(-30.f + i * 3.99f, 0.f, -10.3f);
        m_objectHandler.addObjectInstance("grass", wd);
    }

    wd.Scale = glm::vec3(4.f, 4.f, 8.f);
    wd.Position = glm::vec3(-30.f, 0.f, 0.f);
    wd.Rotation = glm::vec3(0.f, 0.f, 0.f);

    for (int i = 0; i < 30; i++) {
        m_objectHandler.addObjectInstance("street", wd);
        wd.Position = glm::vec3(-30.f + i*3.99f, 0.f, 0.f);
    }

    m_objectHandler.addGeometry("tree", "Models/tree.obj");
    m_objectHandler.addMaterial("treeMat", glm::vec4(0.1f, 0.7f, 0.1f, 1.f), glm::vec3(0.1f, 0.9f, 0.1f), 0.1f);
    m_objectHandler.bindObject("tree", "tree", "", "treeMat");

    wd.Scale = glm::vec3(2.f, 2.f, 2.f);
    wd.Position = glm::vec3(10.f, 1.f, 3.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(2.f, 3.f, 2.f);
    wd.Position = glm::vec3(20.f, 1.f, -4.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(2.f, 4.f, 2.f);
    wd.Position = glm::vec3(-20.f, 1.f, 5.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(3.f, 3.f, 3.f);
    wd.Position = glm::vec3(50.f, 1.f, 6.f);
    m_objectHandler.addObjectInstance("tree", wd);

    wd.Scale = glm::vec3(3.f, 2.f, 3.f);
    wd.Position = glm::vec3(70.f, 1.f, -5.f);
    m_objectHandler.addObjectInstance("tree", wd);*/

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

    glGenTextures(1, &buffers.cubemapTexture.texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, buffers.cubemapTexture.texture);

    int width, height, nrChannels;

    for (unsigned int i = 0; i < cubemapFaces.size(); i++) {
        buffers.cubemapTexture.data = stbi_load(("Textures/Cubemap/" + cubemapFaces[i]).c_str(), &width, &height, &nrChannels, 0);

        if (buffers.cubemapTexture.data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffers.cubemapTexture.data);
            stbi_image_free(buffers.cubemapTexture.data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << cubemapFaces[i] << std::endl;
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
    m_ambientColor = glm::vec3(0.8f, 0.8f, 0.8f);
    Light lit;
    lit.Strength = glm::vec3(0.9f, 0.9f, 0.9f);
    lit.Direction = glm::vec3(-1.f, -3.f, 1.f);
    lit.Type = 1;
    m_lights.push_back(lit);
    lit.Direction = glm::vec3(1.f, -3.f, 1.f);
    m_lights.push_back(lit);
    /*lit.Type = 2;
    lit.FalloffStart = 10.f;
    lit.FalloffEnd = 30.f;
    lit.Strength = glm::vec3(0.7f, 0.7f, 0.7f);
    lit.Position = glm::vec3(0.f, 1.f, 0.1);
    m_lights.push_back(lit);*/
    /*lit.Direction = glm::vec3(-1.f, -3.f, 1.f);
    m_lights.push_back(lit);*/
}

void TSR_Simulation::InitBuffers() {
    //Init all neceserry buffers such as frame buffers, texture buffers, depth buffers...
    InitMaterialsBuffers();
    InitLightBuffers();
    InitPickingBuffers();
    InitSecondViewBuffers();
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, M_SCR_WIDTH, M_SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenRenderbuffers(1, &buffers.secondViewDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, buffers.secondViewDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, M_SCR_WIDTH, M_SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffers.secondViewDepthRBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffers.secondViewTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Error: Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TSR_Simulation::InitShaders() {
    m_shaderHandler.addShaders("standard", "Shaders/VertexShader.glsl", "Shaders/PixelShaderBlinnPhong.glsl");
    m_shaderHandler.addShaders("textured", "Shaders/VertexShader.glsl", "Shaders/PixelShaderTextures.glsl");
   // m_shaderHandler.addShaders("noShading", "Shaders/VertexShader.glsl", "Shaders/PixelShaderLights.glsl");
    m_shaderHandler.useShader("textured");
    InitBuffers();
    m_shaderHandler.linkShaderUniformBlock("textured", "MaterialBlock", 0);
    m_shaderHandler.linkShaderUniformBlock("textured", "LightBlock", 1);
    m_shaderHandler.linkShaderUniformBlock("standard", "MaterialBlock", 0);
    m_shaderHandler.linkShaderUniformBlock("standard", "LightBlock", 1);

    m_shaderHandler.addShaders("picking", "Shaders/VertexShader.glsl", "Shaders/PixelShaderPicking.glsl");
    m_shaderHandler.addShaders("outline", "Shaders/VertexShader.glsl", "Shaders/PixelShaderPickedOutline.glsl");
    m_shaderHandler.addShaders("cubemap", "Shaders/VertexShaderCubemap.glsl", "Shaders/PixelShaderCubemap.glsl");
}

void TSR_Simulation::Update() {
    InputUpdate();
    ClutterUpdate();
    /*if (m_pickedRenderObject->worldData->at(m_pickedObjectIndex).Picked) {
        m_pickedRenderObject->worldData->at(m_pickedObjectIndex).move(0.01f, 0.01f, 0.01f);
    }*/
}

void TSR_Simulation::InputUpdate() {
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraHandlerOuter.moveFront(m_timer.getDeltaTime());
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraHandlerOuter.moveBack(m_timer.getDeltaTime());
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraHandlerOuter.moveLeft(m_timer.getDeltaTime());
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraHandlerOuter.moveRight(m_timer.getDeltaTime());
    if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
        m_cameraHandlerOuter.lookLeft(m_timer.getDeltaTime());
    if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        m_cameraHandlerOuter.lookRight(m_timer.getDeltaTime());
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        m_cameraHandlerOuter.lookUp(m_timer.getDeltaTime());
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        m_cameraHandlerOuter.lookDown(m_timer.getDeltaTime());

    //if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
    //    double mouseX, mouseY;
    //    glfwGetCursorPos(m_window, &mouseX, &mouseY);

    //    if (m_firstMouse) {
    //        m_lastMouseX = mouseX;
    //        m_lastMouseY = mouseY;
    //        m_firstMouse = false;
    //    }

    //    double deltaX = mouseX - m_lastMouseX;
    //    double deltaY = m_lastMouseY - mouseY;

    //    m_cameraHandlerOuter.lookAround(deltaX, deltaY);

    //    m_lastMouseX = mouseX;
    //    m_lastMouseY = mouseY;
    //} else {
    //    m_firstMouse = true;
    //}

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
}

void TSR_Simulation::ClutterUpdate() {
    for (auto& it : m_objectHandler.getObjectsVectorType(ObjectType::CLUTTER)) {
        for (int i = 0; i < it->worldData->size(); i++) {
            if (it->worldData->at(i).Position.x < -30.f) {
                it->worldData->at(i).Position.x = 85.71f;
            } else {
                it->worldData->at(i).Position.x -= 0.01f;
            }
        }
    }
}

void TSR_Simulation::Draw() {
    //Here we'll call all draw pass functions such as the draw pass, picking pass, shadow pass, outline pass...

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    CubemapDrawPass(CameraType::OUTSIDE_CAMERA);
    PickingDrawPass();
    ObjectDrawPass(CameraType::OUTSIDE_CAMERA);
    OutlineDrawPass();

    if (m_timer.getCounter() > 0.2f) {
        glBindFramebuffer(GL_FRAMEBUFFER, buffers.secondViewFBO);
        glViewport(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        CubemapDrawPass(CameraType::INSIDE_CAMERA);
        ObjectDrawPass(CameraType::INSIDE_CAMERA);

        std::shared_ptr<std::vector<unsigned char>> pixels = std::make_shared<std::vector<unsigned char>>((M_SCR_WIDTH * M_SCR_HEIGHT * 3));
        glReadPixels(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels->data());

        std::thread t(std::bind(&TSR_Simulation::saveFboToImage, this, pixels));
        t.detach();

        /*saveFboToImage(pixels);*/

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT);

        m_timer.resetCounter();
    }

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void TSR_Simulation::PickingDrawPass() {
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glDisable(GL_BLEND);
        m_shaderHandler.setMat4x4("picking", "view", m_cameraHandlerOuter.getView());

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
            glBindVertexArray(obj->geometry->VAO);
            for (int j = 0; j < obj->geometry->startIndexies.size(); j++) {

                //This goes into a sub-loop for each instance of an object
                //There is currently only one object with one instance
                for (int i = 0; i < obj->worldData->size(); i++) {
                    m_shaderHandler.setMat4x4("picking", "world", obj->worldData->at(i).getWorldTransform());

                    GLubyte r = ((obj->objectID + 1) & 0xFF);
                    GLubyte g = ((i + 1) & 0xFF);
                    m_shaderHandler.setVec3("picking", "idColor", glm::vec3(r / 255.f, g / 255.f, 0));

                    glDrawElements(GL_TRIANGLES, obj->geometry->numOfIndecies[j], GL_UNSIGNED_INT, (void*)(obj->geometry->startIndexies[j] * sizeof(unsigned int)));
                }

            }
            glBindVertexArray(0);
        }

        GLubyte pixelColor[3];
        glReadPixels(readX, readY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixelColor);

        //std::cout << "\n" << (int)pixelColor[0] << " " << (int)pixelColor[1] << " " << buffers.pickingFBO;
        if ((int)pixelColor[0] != 0 && (int)pixelColor[1] != 0) {
            m_pickedRenderObject->worldData->at(m_pickedObjectIndex).Picked = false;
            m_pickedRenderObject = m_objectHandler.getObjectsVector()[(int)pixelColor[0] - 1];
            m_pickedObjectIndex = (int)pixelColor[1] - 1;
            m_pickedRenderObject->worldData->at(m_pickedObjectIndex).Picked = true;
        } else {
            m_pickedRenderObject->worldData->at(m_pickedObjectIndex).Picked = false;
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

    if (type == CameraType::OUTSIDE_CAMERA) {
        m_shaderHandler.setMat4x4("textured", "view", m_cameraHandlerOuter.getView());
        m_shaderHandler.setVec3("textured", "cameraPos", m_cameraHandlerOuter.getCameraPos());
        m_shaderHandler.setMat4x4("standard", "view", m_cameraHandlerOuter.getView());
        m_shaderHandler.setVec3("standard", "cameraPos", m_cameraHandlerOuter.getCameraPos());
    } else {
        m_shaderHandler.setMat4x4("textured", "view", m_cameraHandlerInner.getView());
        m_shaderHandler.setVec3("textured", "cameraPos", m_cameraHandlerInner.getCameraPos());
        m_shaderHandler.setMat4x4("standard", "view", m_cameraHandlerInner.getView());
        m_shaderHandler.setVec3("standard", "cameraPos", m_cameraHandlerInner.getCameraPos());
    }

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    for (auto& obj : m_objectHandler.getObjectsVector()) {
        if (type == CameraType::INSIDE_CAMERA && obj->Type == ObjectType::CAR) {
            continue;
        }

        if (obj->geometry->textures.size() != 0) {
            ObjectDrawPassTextured(obj);
        } else {
            ObjectDrawPassUntextured(obj);
        }
    }
}

void TSR_Simulation::ObjectDrawPassTextured(std::shared_ptr<RenderObject>& obj) {
    m_shaderHandler.useShader("textured");

    glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindVertexArray(obj->geometry->VAO);
    for (int j = 0; j < obj->geometry->numOfIndecies.size(); j++) {
        for (int i = 0; i < obj->worldData->size(); i++) {

            m_shaderHandler.setMat4x4("textured", "world", obj->worldData->at(i).getWorldTransform());

            if (obj->worldData->at(i).Picked) {
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
            } else {
                glStencilFunc(GL_ALWAYS, 0, 0xFF);
                glStencilMask(0xFF);
            }

            /*glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
            glBindBuffer(GL_UNIFORM_BUFFER, 0);*/

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj->geometry->textures[j].texture);
            m_shaderHandler.setTextureUnit("textured");

            glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &obj->material->at(j));

            glDrawElements(GL_TRIANGLES, obj->geometry->numOfIndecies[j], GL_UNSIGNED_INT, (void*)(obj->geometry->startIndexies[j] * sizeof(unsigned int)));
        }
    }
}

void TSR_Simulation::ObjectDrawPassUntextured(std::shared_ptr<RenderObject>& obj) {
    glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindVertexArray(obj->geometry->VAO);
    for (int j = 0; j < obj->geometry->numOfIndecies.size(); j++) {
        for (int i = 0; i < obj->worldData->size(); i++) {

            if (obj->worldData->at(i).Picked) {
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
            } else {
                glStencilFunc(GL_ALWAYS, 0, 0xFF);
                glStencilMask(0xFF);
            }

            /*glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
            glBindBuffer(GL_UNIFORM_BUFFER, 0);*/
            m_shaderHandler.setMat4x4("standard", "world", obj->worldData->at(i).getWorldTransform());

            glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &obj->material->at(j));

            glDrawElements(GL_TRIANGLES, obj->geometry->numOfIndecies[j], GL_UNSIGNED_INT, (void*)(obj->geometry->startIndexies[j] * sizeof(unsigned int)));
        }
    }
}

void TSR_Simulation::OutlineDrawPass() {
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    m_shaderHandler.setMat4x4("outline", "view", m_cameraHandlerOuter.getView());
    m_shaderHandler.useShader("outline");

    glBindVertexArray(m_pickedRenderObject->geometry->VAO);
    if (m_pickedRenderObject->worldData->at(m_pickedObjectIndex).Picked) {
        for (int j = 0; j < m_pickedRenderObject->geometry->numOfIndecies.size(); j++) {
            m_shaderHandler.setMat4x4("outline", "world", m_pickedRenderObject->worldData->at(m_pickedObjectIndex).getPickedTransform());
            glDrawElements(GL_TRIANGLES, m_pickedRenderObject->geometry->numOfIndecies[j], GL_UNSIGNED_INT, (void*)(m_pickedRenderObject->geometry->startIndexies[j] * sizeof(unsigned int)));
        }
    }

    glEnable(GL_DEPTH_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glDisable(GL_STENCIL_TEST);
}

void TSR_Simulation::CubemapDrawPass(CameraType type) {
    glDepthMask(GL_FALSE);

    if (type == CameraType::OUTSIDE_CAMERA) {
        m_shaderHandler.setMat4x4("cubemap", "view", glm::mat4(glm::mat3(m_cameraHandlerOuter.getView())));
    } else {
        m_shaderHandler.setMat4x4("cubemap", "view", glm::mat4(glm::mat3(m_cameraHandlerInner.getView())));
    }

    glBindVertexArray(buffers.cubemapVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, buffers.cubemapTexture.texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
}

void TSR_Simulation::saveFboToImage(std::shared_ptr<std::vector<unsigned char>> pixels) {

    std::vector<unsigned char> flippedPixels(M_SCR_WIDTH * M_SCR_HEIGHT * 3);
    for (int y = 0; y < M_SCR_HEIGHT; y++) {
        std::memcpy(&flippedPixels[y * M_SCR_WIDTH * 3], &pixels->at((M_SCR_HEIGHT - 1 - y) * M_SCR_WIDTH * 3), M_SCR_WIDTH * 3);
    }

    stbi_write_jpg("testImage.jpg", M_SCR_WIDTH, M_SCR_HEIGHT, 3, flippedPixels.data(), 90);
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
    glDeleteFramebuffers(1, &buffers.pickingFBO);
    glDeleteFramebuffers(1, &buffers.secondViewFBO);
    glDeleteVertexArrays(1, &buffers.cubemapVAO);
    glDeleteBuffers(1, &buffers.cubemapVBO);
}

int TSR_Simulation::Run() {
    m_shaderHandler.setMat4x4("standard", "projection", m_cameraHandlerOuter.getProjection());
    m_shaderHandler.setMat4x4("textured", "projection", m_cameraHandlerOuter.getProjection());
    m_shaderHandler.setMat4x4("picking", "projection", m_cameraHandlerOuter.getProjection());
    m_shaderHandler.setMat4x4("outline", "projection", m_cameraHandlerOuter.getProjection());
    m_shaderHandler.setMat4x4("cubemap", "projection", m_cameraHandlerOuter.getProjection());

    while (!glfwWindowShouldClose(m_window)) {
        m_timer.startTime();

        Update();

        Draw();

        m_timer.update();
    }

    glfwTerminate();

    return 0;
}