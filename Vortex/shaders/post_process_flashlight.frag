#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float uTime;

void main()
{
    vec3 sceneColor = texture(screenTexture, TexCoords).rgb;

    vec2 texSize = textureSize(screenTexture, 0);
    float aspectRatio = texSize.x / texSize.y;

    vec2 correctedCoords = TexCoords;
    correctedCoords.x *= aspectRatio;

    vec2 center = vec2(0.5, 0.5);
    center.x *= aspectRatio;

    center.x += sin(uTime * 1.5) * 0.02;
    center.y += cos(uTime * 2.0) * 0.02;

    float flicker = sin(uTime * 20.0) * 0.005 + cos(uTime * 45.0) * 0.005;
    float innerRadius = 0.15 + flicker; 
    
    float outerRadius = 0.45;
    float edgeDarkness = 0.02;

    float dist = distance(correctedCoords, center);
    float intensity = 1.0 - smoothstep(innerRadius, outerRadius, dist);
    intensity = max(intensity, edgeDarkness);

    vec3 lightTint = mix(vec3(1.0), vec3(1.0, 0.95, 0.85), intensity);

    FragColor = vec4(sceneColor * intensity * lightTint, 1.0);
}
