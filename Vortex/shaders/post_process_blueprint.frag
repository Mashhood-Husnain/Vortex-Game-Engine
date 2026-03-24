#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    float offset = 1.0 / 500.0;
    
    float top    = dot(texture(screenTexture, TexCoords + vec2(0, offset)).rgb, vec3(0.33));
    float bottom = dot(texture(screenTexture, TexCoords - vec2(0, offset)).rgb, vec3(0.33));
    float left   = dot(texture(screenTexture, TexCoords - vec2(offset, 0)).rgb, vec3(0.33));
    float right  = dot(texture(screenTexture, TexCoords + vec2(offset, 0)).rgb, vec3(0.33));
    
    float h = abs(top - bottom);
    float v = abs(left - right);
    float edge = h + v;
    
    vec3 background = vec3(0.1, 0.2, 0.5);
    vec3 color = mix(background, vec3(1.0), step(0.1, edge));
    
    FragColor = vec4(color, 1.0);
}
