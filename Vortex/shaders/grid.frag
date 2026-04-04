#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform vec3 cameraPos;

void main() {
    float dist = length(WorldPos - cameraPos);
    float maxDist = 50.0;
    float fade = max(0.0, 1.0 - (dist / maxDist));

    vec2 coord = WorldPos.xz;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);
    
    vec4 color = vec4(0.5, 0.5, 0.5, 1.0 - min(line, 1.0));

    float xAxis = abs(WorldPos.z) / fwidth(WorldPos.z);
    float zAxis = abs(WorldPos.x) / fwidth(WorldPos.x);
    
    if (xAxis < 1.0) color = vec4(1.0, 0.2, 0.2, 1.0); 
    if (zAxis < 1.0) color = vec4(0.2, 0.3, 1.0, 1.0);

    color.a *= fade;

    if (color.a < 0.01) discard;
    
    FragColor = color;
}
