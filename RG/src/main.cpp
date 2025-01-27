#define GL_SILENCE_DEPRECATION
#include "../../vendor/glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "../../vendor/imgui/imgui.h"
#include "../../vendor/imgui/backends/imgui_impl_glfw.h"
#include "../../vendor/imgui/backends/imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <cmath>
#include "/Users/lukaovsenjak/Downloads/glm/glm.hpp"
#include "/Users/lukaovsenjak/Downloads/glm/gtc/matrix_transform.hpp"
#include "/Users/lukaovsenjak/Downloads/glm/gtc/type_ptr.hpp"
#include <string>

// Basic vertex shader
const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    out vec2 TexCoord;
    out vec3 Normal;
    
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
        
        // Calculate normal based on neighboring vertices
        // This is a simplified normal calculation
        vec3 dx = dFdx(aPos);
        vec3 dy = dFdy(aPos);
        Normal = normalize(cross(dx, dy));
    }
)";

// Basic fragment shader
const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec2 TexCoord;
    in vec3 Normal;
    
    uniform vec3 lightDir;
    
    void main() {
        vec3 waterColor = vec3(0.2, 0.4, 0.8);
        
        // Basic lighting calculation
        float diff = max(dot(Normal, normalize(lightDir)), 0.0);
        vec3 diffuse = diff * waterColor;
        
        // Add specular highlight
        vec3 viewDir = vec3(0.0, 1.0, 0.0);
        vec3 reflectDir = reflect(-normalize(lightDir), Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = vec3(0.5) * spec;
        
        FragColor = vec4(diffuse + specular + waterColor * 0.2, 0.9);
    }
)";

class WaterSurface
{
private:
    GLuint VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int gridSize;
    float time;

    // Shader program
    GLuint shaderProgram;

    void generateGrid()
    {
        // Create a grid of vertices
        for (int z = 0; z < gridSize; z++)
        {
            for (int x = 0; x < gridSize; x++)
            {
                vertices.push_back(x / (float)(gridSize - 1) * 2.0f - 1.0f); // X
                vertices.push_back(0.0f);                                    // Y
                vertices.push_back(z / (float)(gridSize - 1) * 2.0f - 1.0f); // Z
                vertices.push_back(x / (float)(gridSize - 1));               // U
                vertices.push_back(z / (float)(gridSize - 1));               // V
            }
        }

        // Generate indices for triangles
        for (int z = 0; z < gridSize - 1; z++)
        {
            for (int x = 0; x < gridSize - 1; x++)
            {
                int topLeft = z * gridSize + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * gridSize + x;
                int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
    }

    void setupShaders()
    {
        // Create vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        checkShaderCompilation(vertexShader, "vertex");

        // Create fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        checkShaderCompilation(fragmentShader, "fragment");

        // Create shader program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        checkProgramLinking(shaderProgram);

        // Delete shaders as they're no longer needed
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void checkShaderCompilation(GLuint shader, const char *type)
    {
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }
    }

    void checkProgramLinking(GLuint program)
    {
        GLint success;
        GLchar infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                      << infoLog << std::endl;
        }
    }

public:
    WaterSurface(int size = 50) : gridSize(size), time(0.0f)
    {
        generateGrid();
        setupBuffers();
        setupShaders();
    }

    void setupBuffers()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void update(float deltaTime)
    {
        time += deltaTime;

        // Update vertex positions for wave animation
        std::vector<float> newVertices = vertices;
        for (size_t i = 0; i < vertices.size(); i += 5)
        {
            float x = vertices[i];
            float z = vertices[i + 2];

            // Create wave effect using multiple sine waves
            float height = 0.0f;
            height += 0.1f * sin(x * 2.0f + time * 1.5f);
            height += 0.05f * sin(z * 3.0f + time * 2.0f);
            height += 0.03f * sin((x + z) * 2.0f + time);

            newVertices[i + 1] = height; // Update Y coordinate
        }

        // Update vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, newVertices.size() * sizeof(float), newVertices.data());
    }

    void render(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &model)
    {
        glUseProgram(shaderProgram);

        // Set uniforms
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

        // Set light direction (you can modify this as needed)
        glm::vec3 lightDir(0.5f, -1.0f, 0.3f);
        glUniform3fv(lightDirLoc, 1, &lightDir[0]);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }

    ~WaterSurface()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteProgram(shaderProgram);
    }
};

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f);
float cameraSpeed = 2.5f;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Function prototypes
void processInput(GLFWwindow *window);

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS
    glfwWindowHint(GLFW_SAMPLES, 4);                     // Enable multisampling

    // Create window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Water Simulation", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set callbacks

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create water surface
    WaterSurface water(100); // 100x100 grid for better detail

    // Create camera matrices
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input processing
        processInput(window);

        // Update view matrix based on camera position
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));

        // Create model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Move water below camera
        model = glm::scale(model, glm::vec3(4.0f, 1.0f, 4.0f));      // Scale water surface

        // Clear buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update and render water
        water.update(deltaTime);
        water.render(projection, view, model);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwTerminate();
    return 0;
}

// Process keyboard input
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * glm::normalize(cameraTarget - cameraPos);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * glm::normalize(cameraTarget - cameraPos);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraTarget - cameraPos, glm::vec3(0.0f, 1.0f, 0.0f))) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraTarget - cameraPos, glm::vec3(0.0f, 1.0f, 0.0f))) * cameraSpeed;
}

/*
#define GL_SILENCE_DEPRECATION
#include "../../vendor/glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "../../vendor/imgui/imgui.h"
#include "../../vendor/imgui/backends/imgui_impl_glfw.h"
#include "../../vendor/imgui/backends/imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <cmath>
#include "/Users/lukaovsenjak/Downloads/glm/glm.hpp"
#include "/Users/lukaovsenjak/Downloads/glm/gtc/matrix_transform.hpp"
#include "/Users/lukaovsenjak/Downloads/glm/gtc/type_ptr.hpp"
#include <string>
#include "OBJLoader/Model.h"

const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoords;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    out vec3 Normal;
    out vec3 FragPos;

    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

const char *fragmentShaderSource = R"(
    #version 330 core
    in vec3 Normal;
    in vec3 FragPos;

    out vec4 FragColor;

    void main() {
        // Light position and properties
        vec3 lightPos = vec3(0.0, 10.0, 0.0); // Light positioned above
        vec3 lightColor = vec3(1.0, 1.0, 1.0); // White light
        vec3 baseColor = vec3(0.7, 0.7, 0.7); // Lighter gray color

        // Ambient lighting
        float ambientStrength = 0.4;
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse lighting
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular lighting
        float specularStrength = 0.5;
        vec3 viewPos = vec3(0.0, 5.0, 10.0); // Match camera position
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = specularStrength * spec * lightColor;

        // Combine all lighting components
        vec3 result = (ambient + diffuse + specular) * baseColor;
        FragColor = vec4(result, 1.0);
    }
)";

struct GameState
{
    float playerX = 0.0f;
    float playerZ = 0.0f;
    float playerY = 0.5f; // Add height for hovering
    float speed = 0.05f;
    float score = 0.0f;
    bool gameOver = false;
    int lane = 1;
    bool isThirdPerson = true;
    float cameraDistance = 5.0f;
};

std::vector<float> trackSegments = {
    -15.0f, -30.0f, -45.0f, -60.0f, -75.0f};

unsigned int createShaderProgram()
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

Model *roadModel = nullptr;

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(800, 600, "Endless Runner", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 1, 5, 5, 4, 0,
        2, 3, 7, 7, 6, 2,
        0, 3, 7, 7, 4, 0,
        1, 2, 6, 6, 5, 1};

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GameState gameState;
    glEnable(GL_DEPTH_TEST);

    roadModel = new Model("assets/Road.obj");

    Model *grassModel = new Model("../assets/Grass.obj");

    Model *stopSignModel = new Model("../assets/StopSign.obj");

    std::vector<float> grassSegmentsLeft = {
        -15.0f, -30.0f, -45.0f, -60.0f, -75.0f};
    std::vector<float> grassSegmentsRight = {
        -15.0f, -30.0f, -45.0f, -60.0f, -75.0f};

    std::vector<float> stopSignSegments = {
        -30.0f, -60.0f, -120.0f};

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && gameState.lane > 0)
        {
            gameState.lane--;
            gameState.playerX -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && gameState.lane < 2)
        {
            gameState.lane++;
            gameState.playerX += 1.0f;
        }

        if (!gameState.gameOver)
        {
            gameState.playerZ -= gameState.speed;
            gameState.score += gameState.speed;

            for (auto &segmentZ : trackSegments)
            {
                segmentZ += gameState.speed;
                if (segmentZ > 15.0f)
                {
                    float minZ = trackSegments[0];
                    for (float z : trackSegments)
                    {
                        minZ = std::min(minZ, z);
                    }
                    segmentZ = minZ - 15.0f;
                }
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 5.0f));

        if (gameState.isThirdPerson)
        {
            view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 5.0f));
            view = glm::rotate(view, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
        }
        else
        {
            glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 5.0f));
        }
        glm::mat4 projection = glm::perspective(glm::radians(300.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (const auto &segmentZ : trackSegments)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 13.75f, segmentZ - 10.0f));
            model = glm::rotate(model, glm::radians(150.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            stopSignModel->Draw(shaderProgram);
        }

        for (const auto &segmentZ : trackSegments)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, segmentZ));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            roadModel->Draw(shaderProgram);
        }

        glm::mat4 playerModel = glm::translate(glm::mat4(1.0f),
                                               glm::vec3(gameState.playerX, gameState.playerY, gameState.playerZ));
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(playerModel));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Game Stats");
        ImGui::Text("Score: %.1f", gameState.score);
        ImGui::Text("Speed: %.2f", gameState.speed);
        if (ImGui::Checkbox("Third Person View", &gameState.isThirdPerson))
        {
            if (gameState.isThirdPerson)
            {
                gameState.cameraDistance = 5.0f;
            }
        }

        if (ImGui::Button("Reset Game"))
        {
            gameState = GameState();
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    delete roadModel;

    return 0;
}

/*
mkdir build
cd build
cmake ..
make
./VehicleSimulator


*/
