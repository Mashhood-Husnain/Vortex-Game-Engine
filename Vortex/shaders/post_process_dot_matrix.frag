#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    float dotSize = 80.0; 
    vec2 gridUv = fract(TexCoords * dotSize);
    
    vec2 sampleUv = floor(TexCoords * dotSize) / dotSize;
    vec3 color = texture(screenTexture, sampleUv).rgb;
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));

    float dist = distance(gridUv, vec2(0.5));
    float circle = step(dist, luminance * 0.5);

    FragColor = vec4(vec3(circle), 1.0);
}
