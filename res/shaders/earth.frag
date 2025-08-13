#version 330 core
in vec2 UV;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D earthTexture;
uniform vec3 lightPos; // Позиция источника света (в мировых координатах)
uniform vec3 lightColor;


uniform float ambientStrength;
uniform float specularStrength;
uniform vec3 viewPos;
//uniform vec3 objectColor;

void main() {
    // Ambient освещение (фоновый свет)
    //float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Диффузное освещение
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Спекулярное освещение
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * texture(earthTexture, UV).rgb;
    FragColor = vec4(result, 1.0);
}