#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    float resX = 320.0;
    float resY = 240.0;
    vec2 uv = vec2(floor(TexCoords.x * resX) / resX, floor(TexCoords.y * resY) / resY);
    
    vec3 col = texture(screenTexture, uv).rgb;
    
    float levels = 8.0; 
    col = floor(col * levels) / levels;
    
    col *= 1.1;

    FragColor = vec4(col, 1.0);
}
