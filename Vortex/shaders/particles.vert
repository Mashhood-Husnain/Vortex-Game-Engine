#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 instancePos;
layout (location = 2) in float aSize;
layout (location = 3) in vec4 aColor;

out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 scaledPos = aPos * aSize;
    
    vec3 billboardPos = instancePos + cameraRight * scaledPos.x + cameraUp * scaledPos.y;
    
    ParticleColor = aColor;
    gl_Position = projection * view * vec4(billboardPos, 1.0);
}
