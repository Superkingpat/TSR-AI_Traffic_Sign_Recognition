#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;
in vec3 fragColor;

uniform vec3 materialColor;

void main() {
    FragColor = vec4(materialColor, 1.0);
    float gamma = 2.2;
    FragColor.rgb = pow(fragColor.rgb, vec3(1.0 / gamma));
    FragColor.a = 1.0;
}