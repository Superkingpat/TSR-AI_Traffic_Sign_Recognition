#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

void main() {
    FragColor = vec4(0.f, 1.f, 0.f, 1.f);
}