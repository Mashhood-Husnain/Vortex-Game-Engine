#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D u_diffuseMap;
uniform sampler2D u_roughnessMap;
uniform sampler2D u_metallicMap;
// uniform sampler2D u_normalMap; // will be added later for better lighting

uniform vec3 lightPos; // light position in world space
uniform vec3 viewPos; // camera position in world space
uniform bool u_hasTexture;
uniform bool u_hasRoughness;
uniform bool u_hasMetallic;

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
    vec3 ambient = 0.15 * diffuseCol;

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

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
