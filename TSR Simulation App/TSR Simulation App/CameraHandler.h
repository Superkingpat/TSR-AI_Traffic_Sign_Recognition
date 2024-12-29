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

class CameraHandler
{
};

