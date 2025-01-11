#version 440 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec2 TexCoords;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform mat4 world;
uniform mat4 projectionView;
uniform mat4 lightSpaceMatrix;

void main() {
    FragPos = vec3(world * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(world))) * aNormal; 
    TexCoords = aTexCoords;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    gl_Position = projectionView * world * vec4(aPos, 1.0);
}
