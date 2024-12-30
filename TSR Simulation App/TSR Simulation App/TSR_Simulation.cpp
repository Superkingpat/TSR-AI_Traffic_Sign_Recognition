#include "TSR_Simulation.h"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void TSR_Simulation::Init() {
    InitGLFW();
    InitCamera();
    InitOpenGL();
    InitRenderObjects();
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
    conf.pitch = 20.f;
    conf.yaw = 0.f;
    conf.speed = 10.f;
    m_cameraHandler = CameraHandler(conf);
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

    m_window = glfwCreateWindow(M_SCR_WIDTH, M_SCR_HEIGHT, "TSR Simulation", NULL/*glfwGetPrimaryMonitor()*/, NULL);

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
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0x00);
}

void TSR_Simulation::InitRenderObjects() {
    m_objectHandler.addGeometry("monkey", "Models/monkey.obj");
    m_objectHandler.addGeometry("dragon", "Models/dragon2.obj");
    m_objectHandler.addMaterial("testMat", glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec3(1.f, 0.f, 0.f), 0.5f);
    m_objectHandler.addMaterial("testMat2", glm::vec4(0.5f, 0.5f, 0.5f, 1.f), glm::vec3(0.5f, 0.5f, 0.5f), 0.8f);
    m_objectHandler.bindObject("monkey", "monkey", "", "testMat");
    m_objectHandler.bindObject("dragon", "dragon", "", "testMat2");

    WorldData wd;
    wd.Picked = false;
    wd.Position = glm::vec3(2.f, 4.f, 0.f);
    wd.Rotation = glm::vec3(90.f, 0.f, 0.f);
    wd.Scale = glm::vec3(2.f, 2.f, 2.f);

    m_objectHandler.addObjectInstance("monkey", wd);

    wd.Position = glm::vec3(0.f, 0.f, 0.f);
    wd.Rotation = glm::vec3(0.f, 0.f, 0.f);

    m_objectHandler.addObjectInstance("dragon", wd);
}

void TSR_Simulation::InitLights() {
    Light lit;
    lit.Type = 1;
    m_lights.push_back(lit);
}

void TSR_Simulation::InitBuffers() {
    //Init all neceserry buffers such as frame buffers, texture buffers, depth buffers...
    InitMaterialsBuffers();
    InitLightBuffers();
    InitPickingBuffers();
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

    std::cout << "Buff: " << buffers.pickingFBO;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TSR_Simulation::InitShaders() {
    m_shaderHandler.addShaders("standard", "Shaders/VertexShader.glsl", "Shaders/PixelShaderBlinnPhong.glsl");
   // m_shaderHandler.addShaders("noShading", "Shaders/VertexShader.glsl", "Shaders/PixelShaderLights.glsl");
    m_shaderHandler.useShader("standard");
    InitBuffers();
    m_shaderHandler.linkShaderUniformBlock("standard", "MaterialBlock", 0);
    m_shaderHandler.linkShaderUniformBlock("standard", "LightBlock", 1);
    m_shaderHandler.addShaders("picking", "Shaders/VertexShader.glsl", "Shaders/PixelShaderPicking.glsl");
    //m_shaderHandler.addShaders("outline", "Shaders/VertexShader.glsl", "Shaders/PixelShaderPickedOutline.glsl");
}

void TSR_Simulation::Update() {
    InputUpdate();
}

void TSR_Simulation::InputUpdate() {
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraHandler.moveFront(0.01f);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraHandler.moveBack(0.01f);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraHandler.moveLeft(0.01f);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraHandler.moveRight(0.01f);
    if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
        m_cameraHandler.lookLeft(0.01f);
    if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        m_cameraHandler.lookRight(0.01f);
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        m_cameraHandler.lookUp(0.01f);
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        m_cameraHandler.lookDown(0.01f);

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
}

void TSR_Simulation::Draw() {
    //Here we'll call all draw pass functions such as the draw pass, picking pass, shadow pass, outline pass...

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    PickingDrawPass();
    ObjectDrawPass();
    OutlineDrawPass();
    CubemapDrawPass();

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void TSR_Simulation::PickingDrawPass() {
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        m_shaderHandler.setMat4x4("picking", "view", m_cameraHandler.getView());

        double mouseX, mouseY;
        glfwGetCursorPos(m_window, &mouseX, &mouseY);

        int readX = static_cast<int>(mouseX);
        int readY = M_SCR_HEIGHT - static_cast<int>(mouseY) - 1;

        glBindFramebuffer(GL_FRAMEBUFFER, buffers.pickingFBO);
        glViewport(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT);

        glDisable(GL_MULTISAMPLE);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        //This goes into a loop for each object
        for (const auto& obj : m_objectHandler.getObjectsVector()) {
            glBindVertexArray(obj->geometry->VAO);

            //This goes into a sub-loop for each instance of an object
            //There is currently only one object with one instance
            for (int i = 0; i < obj->worldData->size(); i++) {
                m_shaderHandler.setMat4x4("picking", "world", obj->worldData->at(i).getWorldTransform());

                GLubyte r = ((obj->objectID + 1) & 0xFF);
                GLubyte g = ((i + 1) & 0xFF);
                m_shaderHandler.setVec3("picking", "idColor", glm::vec3(r / 255.f, g / 255.f, 0));

                glDrawElements(GL_TRIANGLES, obj->geometry->indecies.size(), GL_UNSIGNED_INT, 0);
            }

            glBindVertexArray(0);
        }

        GLubyte pixelColor[3];
        glReadPixels(readX, readY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixelColor);

        std::cout << "\n" << (int)pixelColor[0] << " " << (int)pixelColor[1] << " " << buffers.pickingFBO;
        /*if ((int)pixelColor[0] != 0 && (int)pixelColor[1] != 0) {
            pickedItem.item = (int)pixelColor[0] - 1;
            pickedItem.instance = (int)pixelColor[1] - 1;
        }*/

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, M_SCR_WIDTH, M_SCR_HEIGHT);
        glEnable(GL_MULTISAMPLE);
    }
}

void TSR_Simulation::ObjectDrawPass() {
    m_shaderHandler.setMat4x4("standard", "view", m_cameraHandler.getView());
    m_shaderHandler.setInt("standard", "numOfLights", m_lights.size());
    m_shaderHandler.setVec3("standard", "ambientColor", m_ambientColor);
    m_shaderHandler.setVec3("standard", "cameraPos", m_cameraHandler.getCameraPos());

    for (auto& obj : m_objectHandler.getObjectsVector()) {
        glBindBuffer(GL_UNIFORM_BUFFER, buffers.lightsUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Light) * m_lights.size(), m_lights.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindVertexArray(obj->geometry->VAO);
        m_shaderHandler.setMat4x4("standard", "world", obj->worldData->at(0).getWorldTransform());

        glBindBuffer(GL_UNIFORM_BUFFER, buffers.materialUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), obj->material.get());

        glDrawElements(GL_TRIANGLES, obj->geometry->indecies.size(), GL_UNSIGNED_INT, 0);
    }
}

void TSR_Simulation::OutlineDrawPass() {

}

void TSR_Simulation::CubemapDrawPass() {

}

TSR_Simulation::TSR_Simulation() {
    Init();
}

TSR_Simulation::~TSR_Simulation() {
    glDeleteBuffers(1, &buffers.materialUBO);
    glDeleteBuffers(1, &buffers.lightsUBO);
    glDeleteTextures(1, &buffers.pickingTexture);
    glDeleteFramebuffers(1, &buffers.pickingFBO);
}

int TSR_Simulation::Run() {
    m_shaderHandler.setMat4x4("standard", "projection", m_cameraHandler.getProjection());
    m_shaderHandler.setMat4x4("picking", "projection", m_cameraHandler.getProjection());

    while (!glfwWindowShouldClose(m_window)) {

        Update();

        Draw();
    }

    glfwTerminate();

    return 0;
}