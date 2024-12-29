#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

uniform vec3 materialColor;

void main() {
    FragColor = vec4(materialColor, 1.0);
}