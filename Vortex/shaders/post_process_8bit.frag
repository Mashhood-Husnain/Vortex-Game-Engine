#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;

void main() {
    vec3 col = texture(screenTexture, TexCoords).rgb;
    float numColors = 4.0;
    col = floor(col * numColors) / numColors;
    FragColor = vec4(col, 1.0);
}
