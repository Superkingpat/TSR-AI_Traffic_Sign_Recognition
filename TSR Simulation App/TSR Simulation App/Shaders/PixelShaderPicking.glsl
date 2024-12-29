#version 330 core

out vec3 FragColor;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

uniform vec3 idColor;

void main() {
    FragColor = idColor;
}