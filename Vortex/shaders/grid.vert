#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

out vec3 WorldPos;

void main() {
    vec3 scaledPos = aPos * 100.0;
    
    vec3 snappedPos = vec3(scaledPos.x + cameraPos.x, 0.0, scaledPos.z + cameraPos.z);
    
    WorldPos = snappedPos;
    gl_Position = projection * view * vec4(snappedPos, 1.0);
}
