#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;

void main() {
    float offset = 1.0 / 300.0; // Blur strength
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), vec2( 0.0,    offset), vec2( offset,  offset),
        vec2(-offset,  0.0),    vec2( 0.0,    0.0),    vec2( offset,  0.0),
        vec2(-offset, -offset), vec2( 0.0,   -offset), vec2( offset, -offset)
    );
    float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    );
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) col += vec3(texture(screenTexture, TexCoords.st + offsets[i])) * kernel[i];
    FragColor = vec4(col, 1.0);
}
