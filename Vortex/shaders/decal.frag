#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D u_diffuseMap;
uniform vec2 u_textureScale;
uniform vec3 viewPos;

#define MAX_LIGHTS 4
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform float ambientStrength[MAX_LIGHTS];
uniform int numLights;

uniform float constantFalloff;
uniform float linearFalloff;
uniform float quadraticFalloff;

void main()
{
    vec4 texColor = texture(u_diffuseMap, TexCoords * u_textureScale);
    if(texColor.a < 0.1) discard;

    vec3 norm = normalize(Normal);
    vec3 result = vec3(0.0);

    for(int i = 0; i < numLights; i++)
    {
        float distance = length(lightPos[i] - FragPos);
        float attenuation = 1.0 / (constantFalloff + linearFalloff * distance + quadraticFalloff * (distance * distance));

        vec3 ambient = ambientStrength[i] * lightColor[i];

        vec3 lightDir = normalize(lightPos[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor[i];

        float specularStrength = 0.1;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor[i];

        result += (ambient + diffuse + specular) * attenuation;
    }

    FragColor = vec4(result * texColor.rgb, texColor.a);
}