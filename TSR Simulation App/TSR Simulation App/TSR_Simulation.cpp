#include "TSR_Simulation.h"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void TSR_Simulation::Init() {
    InitGLFW();
    InitCamera();
    InitOpenGL();
    InitRenderObjects();
    InitBuffers();
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
    m_objectHandler.addMaterial("testMat", glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec3(1.f, 0.f, 0.f), 0.5f);
    m_objectHandler.bindObject("monkey", "monkey", "", "testMat");
}

void TSR_Simulation::InitBuffers() {
    //Init all neceserry buffers such as frame buffers, texture buffers, depth buffers...
}

void TSR_Simulation::InitShaders() {
    m_shaderHandler.addShaders("standard", "Shaders/VertexShader.glsl", "Shaders/PixelShaderBlinnPhong.glsl");
    m_shaderHandler.addShaders("noShading", "Shaders/VertexShader.glsl", "Shaders/PixelShaderLights.glsl");
    m_shaderHandler.useShader("standard");
    //createBuffers();
    m_shaderHandler.linkShaderUniformBlock("standard", "MaterialBlock", 0);
    m_shaderHandler.linkShaderUniformBlock("standard", "LightBlock", 1);
    m_shaderHandler.addShaders("picking", "Shaders/VertexShader.glsl", "Shaders/PixelShaderPicking.glsl");
    m_shaderHandler.addShaders("outline", "Shaders/VertexShader.glsl", "Shaders/PixelShaderPickedOutline.glsl");
}

void TSR_Simulation::Update() {

}

void TSR_Simulation::Draw() {
    //Here we'll call all draw pass functions such as the draw pass, picking pass, shadow pass, outline pass...
}

void TSR_Simulation::DrawCubemap() {

}

TSR_Simulation::TSR_Simulation() {
    Init();
}

int TSR_Simulation::Run() {

    m_shaderHandler.setMat4x4("noShading", "projection", m_cameraHandler.getProjection());

    while (!glfwWindowShouldClose(m_window)) {

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

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        m_shaderHandler.setMat4x4("noShading", "world", glm::mat4x4(1.f));
        m_shaderHandler.setMat4x4("noShading", "view", m_cameraHandler.getView());
        m_shaderHandler.setVec3("noShading", "materialColor", m_objectHandler.getObject("monkey").material->Fresnel);

        glBindVertexArray(m_objectHandler.getObject("monkey").geometry->VAO);
        glDrawElements(GL_TRIANGLES, m_objectHandler.getObject("monkey").geometry->indecies.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}