#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float uTime;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main() {
    vec2 uv = TexCoords;
    
    float r = texture(screenTexture, uv + vec2(0.001, 0.0)).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - vec2(0.001, 0.0)).b;
    vec3 col = vec3(r, g, b);

    float noise = (random(uv + uTime) - 0.5) * 0.12;
    col += noise;

    float scanline = sin(uv.y * 600.0) * 0.04;
    col -= scanline;

    float dist = distance(uv, vec2(0.5));
    col *= smoothstep(0.8, 0.3, dist);

    FragColor = vec4(col, 1.0);
}
