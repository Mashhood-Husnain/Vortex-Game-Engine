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
    
    vec3 sceneCol = texture(screenTexture, uv).rgb;
    
    float brightness = dot(sceneCol, vec3(0.299, 0.587, 0.114));
    vec3 nvColor = vec3(0.1, 0.8, 0.2) * brightness;
    
    float noise = (random(uv + uTime) - 0.5) * 0.2;
    nvColor += noise;
    
    float dist = distance(uv, vec2(0.5));
    float vignette = smoothstep(0.7, 0.2, dist);
    nvColor *= vignette;
    
    nvColor -= sin(uv.y * 500.0 + uTime * 5.0) * 0.02;

    FragColor = vec4(nvColor, 1.0);
}
