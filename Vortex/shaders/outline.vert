#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float outline_thickness;

void main()
{
    vec4 world_pos = model * vec4(aPos, 1.0);

    mat3 normal_matrix = mat3(transpose(inverse(model)));
    vec3 world_normal = normalize(normal_matrix * aNormal);

    vec3 fat_world_pos = world_pos.xyz + (world_normal * outline_thickness);

    gl_Position = projection * view * vec4(fat_world_pos, 1.0);
}