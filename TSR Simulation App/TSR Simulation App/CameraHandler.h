#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <map>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
#include <memory>
#include <chrono>
#include <thread>
#include <functional>

#include <stb_image.h>

#include <stb_image_write.h>

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF


struct CameraConfig {
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::mat4 projection;
    float yaw;
    float pitch;
    float sensitivity;
    float speed;
};

class CameraHandler {
private:
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::mat4 projection;

    float yaw;
    float pitch;
    float sensitivity;
    float speed;
public:
    CameraHandler(const CameraConfig& cameraConfig);
    CameraHandler() = default;

    void moveFront(float dt);
    void moveBack(float dt);
    void moveLeft(float dt);
    void moveRight(float dt);

    void lookUp(float dt);
    void lookUp(float amount, float dt);
    void lookDown(float dt);
    void lookDown(float amount, float dt);
    void lookLeft(float dt);
    void lookLeft(float amount, float dt);
    void lookRight(float dt);
    void lookRight(float amount, float dt);

    glm::mat4x4 getView();
    glm::mat4x4 getProjection() const;
    glm::vec3 getCameraPos() const;
};

