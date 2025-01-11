#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 projectionView;
uniform mat4 world;

void main() {
    gl_Position = projectionView * world * vec4(aPos, 1.0);
}
