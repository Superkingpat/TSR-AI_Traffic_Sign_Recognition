#include "ShaderHandler.h"

std::string ShaderHandler::readShader(const std::string& path) {
    std::ifstream shaderFile(path);
    std::stringstream shaderStream;

    if (shaderFile) {
        shaderStream << shaderFile.rdbuf();
    }
    else {
        std::cout << "Error: Could not open shader file: " << path << std::endl;
    }

    shaderFile.close();
    return shaderStream.str();
}

ShaderHandler::~ShaderHandler() {
    for (const auto& it : shaderProgram) {
        glDeleteProgram(it.second);
    }
}

void ShaderHandler::addShaders(const std::string& shaderName, const std::string& vertexShaderSourcePath, const std::string& fragmentShaderSourcePath) {
    std::string vertexShaderSourceStr = readShader(vertexShaderSourcePath);
    std::string fragmentShaderSourceStr = readShader(fragmentShaderSourcePath);
    const char* vertexShaderSource = vertexShaderSourceStr.c_str();
    const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();


    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        throw std::runtime_error(infoLog);
    }


    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        throw std::runtime_error(infoLog);
    }


    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        throw std::runtime_error(infoLog);
    }

    this->shaderProgram[shaderName] = shaderProgram;

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void ShaderHandler::useShader(const std::string& shaderProgramName) {
    GLuint program = shaderProgram[shaderProgramName];
    if (program != currentProgram) {
        glUseProgram(program);
        currentProgram = program;
    }
}

void ShaderHandler::setInt(const std::string& shaderProgramName, const std::string& varName, const int& value) {
    useShader(shaderProgramName);
    GLint location = glGetUniformLocation(shaderProgram[shaderProgramName], varName.c_str());
    glUniform1i(location, value);
}

void ShaderHandler::setFloat(const std::string& shaderProgramName, const std::string& varName, const float& value) {
    useShader(shaderProgramName);
    GLint location = glGetUniformLocation(shaderProgram[shaderProgramName], varName.c_str());
    glUniform1f(location, value);
}

void ShaderHandler::setBool(const std::string& shaderProgramName, const std::string& varName, const bool& value) {
    useShader(shaderProgramName);
    GLint location = glGetUniformLocation(shaderProgram[shaderProgramName], varName.c_str());
    setInt(shaderProgramName, varName, static_cast<int>(value));
}

void ShaderHandler::setVec2(const std::string& shaderProgramName, const std::string& varName, const glm::vec2& value) {
    useShader(shaderProgramName);
    GLint location = glGetUniformLocation(shaderProgram[shaderProgramName], varName.c_str());
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void ShaderHandler::setVec3(const std::string& shaderProgramName, const std::string& varName, const glm::vec3& value) {
    useShader(shaderProgramName);
    GLint location = glGetUniformLocation(shaderProgram[shaderProgramName], varName.c_str());
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderHandler::setVec4(const std::string& shaderProgramName, const std::string& varName, const glm::vec4& value) {
    useShader(shaderProgramName);
    GLint location = glGetUniformLocation(shaderProgram[shaderProgramName], varName.c_str());
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void ShaderHandler::setMat4x4(const std::string& shaderProgramName, const std::string& varName, const glm::mat4x4& value) {
    useShader(shaderProgramName);
    GLint location = glGetUniformLocation(shaderProgram[shaderProgramName], varName.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderHandler::linkShaderUniformBlock(const std::string& shaderProgramName, const std::string& varName, const uint32_t& regNum) {
    useShader(shaderProgramName);
    GLuint materialBlockIndex = glGetUniformBlockIndex(shaderProgram[shaderProgramName], varName.c_str());
    glUniformBlockBinding(shaderProgram[shaderProgramName], materialBlockIndex, regNum);
}

void ShaderHandler::setTextureUnit(const std::string& shaderProgramName) {
    glUniform1i(glGetUniformLocation(shaderProgram[shaderProgramName], "texture_diffuse"), 0);
}
