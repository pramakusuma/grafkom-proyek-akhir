#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

in vec3 Normal; 
in vec3 FragPos;

// Texture samplers
uniform sampler2D ourTexture;

uniform vec3 lightPos;  
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{

    // ambient
    //float ambientStrength = 0.3;
    //vec3 ambient = ambientStrength * lightColor;

    // diffuse 
	float diffuseStrength = 0.4;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = diffuseStrength * max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor ;

    FragColor = vec4(diffuse, 1.0);
    //FragColor = texture(ourTexture, TexCoord);
}