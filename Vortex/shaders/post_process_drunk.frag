#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform float uTime;

void main() {
    vec2 uv = TexCoords;
    uv.x += sin(uv.y * 10.0 + uTime) * 0.01;
    uv.y += cos(uv.x * 10.0 + uTime) * 0.01;
    FragColor = texture(screenTexture, uv);
}
