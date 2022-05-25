#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;
in vec2 textura;
 
uniform vec3 viewPos; 
uniform vec3 lightPos; 
uniform vec3 lightDir; 
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D textureS1;
uniform sampler2D textureS2;

void main()
{
    
    vec3 ld = normalize(lightDir-lightPos);
    vec3 fd = normalize(vec3(FragPos-lightPos));

    // ambient
    float ambientI = 0.5;
    vec3 ambient = ambientI * lightColor;
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = 0.5*diff * lightColor;

	//Specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
	vec3 specular = specularStrength * spec * lightColor;
    
   	vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0)* mix(texture(textureS1, textura), texture(textureS2, textura), 0.6);
   //FragColor = vec4(1.0);
} 