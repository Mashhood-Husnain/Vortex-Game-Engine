#version 330 core
out vec4 FragColor;

in vec4 ParticleColor;
in vec2 TexCoords;

uniform sampler2D particleTexture;
uniform bool useTexture;

void main()
{
    if (useTexture)
    {
        vec4 texColor = texture(particleTexture, TexCoords);
        FragColor = texColor * ParticleColor;
    }
    else
    {
        vec2 center = TexCoords - vec2(0.5);
        float dist = length(center);

        if (dist > 0.5)
        {
            discard;
        }

        float alpha = smoothstep(0.5, 0.3, dist);

        FragColor = vec4(ParticleColor.rgb, ParticleColor.a * alpha);
    }
}