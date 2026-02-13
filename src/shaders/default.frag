#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec4 u_Color;
uniform vec3 lightPos; // light position in world space
uniform vec3 viewPos; // camera position in world space

void main()
{
    // Ambient lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * u_Color.rgb;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_Color.rgb;

    // combine results
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}
