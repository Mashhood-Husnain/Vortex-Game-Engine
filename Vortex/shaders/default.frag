#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D u_diffuseMap;
uniform sampler2D u_roughnessMap;
uniform sampler2D u_metallicMap;
// uniform sampler2D u_normalMap; // will be added later for better lighting

uniform sampler2D shadowMap;
uniform vec3 lightPos; // light position in world space
uniform vec3 viewPos; // camera position in world space
uniform bool u_hasTexture;
uniform bool u_hasRoughness;
uniform bool u_hasMetallic;

uniform vec3 u_fogColor; // temp fog color
uniform float u_fogDensity; // temp fog density

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // Perspective divide and transform to [0,1] range
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Get depth from the shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    // Use a small bias to prevent "shadow acne"
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    // If we are outside the light's frustum, keep it unshadowed
    if(projCoords.z > 1.0) shadow = 0.0;

    return shadow;
}

float random(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 diffuseCol = vec3(0.5); // Default grey
    float roughness = 0.8;       // Default rough
    float metallic  = 0.0;       // Default non-metal

    if (u_hasTexture)
    {
        diffuseCol = texture(u_diffuseMap, TexCoords).rgb;
    }

    if (u_hasRoughness)
    {
        roughness = texture(u_roughnessMap, TexCoords).r;
    }

    if (u_hasMetallic)
    {
        metallic = texture(u_metallicMap, TexCoords).r;
    }

    // ambient lighting
    vec3 ambient = 0.5 * diffuseCol;

    // basic lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffuseCol;

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (1.0 - roughness) * 128.0);
    vec3 specular = spec * mix(vec3(0.3), diffuseCol, metallic);

    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    lighting *= diffuseCol;    
    lighting += (random(TexCoords) - 0.5) * (1.0 / 255.0);

    FragColor = vec4(lighting, 1.0);
}
