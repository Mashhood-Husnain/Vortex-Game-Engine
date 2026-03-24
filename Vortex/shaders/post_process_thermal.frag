#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec3 pix = texture(screenTexture, TexCoords).rgb;
    float lum = dot(pix, vec3(0.299, 0.587, 0.114));
    
    vec3 thermal;
    thermal.r = clamp( (lum - 0.35) / 0.3, 0.0, 1.0) + clamp((lum - 0.85) / 0.3, 0.0, 1.0);
    thermal.g = clamp( (lum - 0.15) / 0.3, 0.0, 1.0) - clamp((lum - 0.65) / 0.3, 0.0, 1.0);
    thermal.b = 1.0 - clamp( (lum - 0.35) / 0.3, 0.0, 1.0);
    
    FragColor = vec4(thermal, 1.0);
}
