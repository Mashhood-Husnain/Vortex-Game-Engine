#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float uTime;

float random(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = TexCoords;
    
    float jumpChance = random(vec2(floor(uTime * 10.0), 0.0));
    if (jumpChance > 0.95) // 5% chance every 10th of a second
    {
        float tearOffset = (random(vec2(uv.y * 10.0, uTime)) - 0.5) * 0.04;
        uv.x += tearOffset;
    }

    vec3 sceneColor = texture(screenTexture, uv).rgb;

    float flicker = random(vec2(uTime * 15.0, 0.0));
    flicker = mix(0.7, 1.0, flicker); 
    sceneColor *= flicker;

    float scratchChance = random(vec2(uv.x * 50.0, floor(uTime * 12.0)));
    if (scratchChance > 0.998) 
    {
        sceneColor += vec3(random(vec2(uTime, uv.y)) * 0.3);
    }

    vec2 texSize = textureSize(screenTexture, 0);
    float aspectRatio = texSize.x / texSize.y;
    
    vec2 center = vec2(0.5, 0.5);
    vec2 correctedCoords = uv;
    correctedCoords.x *= aspectRatio;
    vec2 correctedCenter = center;
    correctedCenter.x *= aspectRatio;

    float dist = distance(correctedCoords, correctedCenter);

    float vignette = smoothstep(1.10, 0.45, dist); 
    sceneColor *= vignette;

    vec3 sepiaTint = vec3(0.9, 0.75, 0.55);
    
    float gray = dot(sceneColor, vec3(0.299, 0.587, 0.114));
    vec3 tintedColor = vec3(gray) * sepiaTint;

    sceneColor = mix(sceneColor, tintedColor, 0.6);

    FragColor = vec4(sceneColor, 1.0);
}
