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

const vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.09418410, -0.92938870 ), vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.65476066, -0.05113977 ), vec2( -0.43095612, -0.81619640 ), 
   vec2( 0.48720372, -0.03590583 ), vec2( -0.21140315, -0.17570340 ), 
   vec2( 0.73030536, 0.61286690 ), vec2( -0.08405104, 0.87169447 ) 
);

float random(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) return 0.0;

    float cosTheta = dot(normal, lightDir);
    float bias = max(0.0005 * (1.0 - cosTheta), 0.00005);
    
    // Normal Offset to kill the last of the acne
    vec3 shiftedPos = projCoords + (normal * 0.0008);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    float angle = random(FragPos.xy) * 6.283185; // 2 * PI
    float s = sin(angle);
    float c = cos(angle);
    mat2 rotation = mat2(c, -s, s, c);

    for (int i = 0; i < 16; i++)
    {
        // Rotate and scale the poisson sample
        vec2 offset = (rotation * poissonDisk[i]) * texelSize * 1.5; 
        float pcfDepth = texture(shadowMap, shiftedPos.xy + offset).r;
        shadow += shiftedPos.z - bias > pcfDepth ? 1.0 : 0.0;
    }

    return shadow / 16.0;
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

    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    lighting *= diffuseCol;    
    lighting += (random(TexCoords) - 0.5) * (1.0 / 255.0);

    FragColor = vec4(lighting, 1.0);
}
