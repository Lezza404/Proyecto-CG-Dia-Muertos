#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct Material {
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 direction;
   
   vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform sampler2D texture_diffuse1;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    vec3 texColor = vec3(texture(texture_diffuse1, TexCoords));
    if (length(texColor) < 0.01) {
        texColor = vec3(0.75, 0.65, 0.60);
    }

    // ambient
    vec3 ambient = light.ambient * texColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texColor;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(material.shininess, 1.0));
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}