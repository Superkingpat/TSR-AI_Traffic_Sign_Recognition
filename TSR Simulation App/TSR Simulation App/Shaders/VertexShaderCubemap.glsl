#version 330 core
layout (location = 0) in vec3 Pos;

out vec3 TexCoords;

uniform mat4 projectionView;

void main() {
    TexCoords = Pos;
    vec4 tempPos = projectionView * vec4(Pos, 1.0);
    gl_Position = tempPos.xyww;
}  