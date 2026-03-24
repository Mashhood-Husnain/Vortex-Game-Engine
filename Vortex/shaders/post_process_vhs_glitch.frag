#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float uTime;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec2 uv = TexCoords;

    float glitchLine = step(0.98, sin(uv.y * 10.0 + uTime * 5.0));
    float drift = glitchLine * 0.05 * sin(uTime * 10.0);
    uv.x += drift;

    float shift = 0.005 * sin(uTime * 2.0);
    float r = texture(screenTexture, uv + vec2(shift, 0.0)).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - vec2(shift, 0.0)).b;

    float noise = rand(uv + vec2(uTime)) * 0.1;
    
    FragColor = vec4(vec3(r, g, b) + noise, 1.0);
}
