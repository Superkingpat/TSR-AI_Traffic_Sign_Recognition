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

class ShaderHandler {
private:
    std::map<std::string, GLuint> shaderProgram;
    GLuint currentProgram = 0;

    std::string readShader(const std::string& path);
public:
    ShaderHandler() = default;
    ~ShaderHandler();
    void addShaders(const std::string& shaderName, const std::string& vertexShaderSourcePath, const std::string& fragmentShaderSourcePath);
    void useShader(const std::string& shaderProgramName);

    void setInt(const std::string& shaderProgramName, const std::string& varName, const int& value);
    void setFloat(const std::string& shaderProgramName, const std::string& varName, const float& value);
    void setBool(const std::string& shaderProgramName, const std::string& varName, const bool& value);
    void setVec2(const std::string& shaderProgramName, const std::string& varName, const glm::vec2& value);
    void setVec3(const std::string& shaderProgramName, const std::string& varName, const glm::vec3& value);
    void setVec4(const std::string& shaderProgramName, const std::string& varName, const glm::vec4& value);
    void setMat4x4(const std::string& shaderProgramName, const std::string& varName, const glm::mat4x4& value);
    void linkShaderUniformBlock(const std::string& shaderProgramName, const std::string& varName, const uint32_t& regNum);
};

