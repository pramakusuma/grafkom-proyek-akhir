#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;


//struct Material {
//    vec3 ambient;
//    vec3 diffuse;
///   vec3 specular;
//    float shininess;
//}; 
  
//uniform Material material;
//struct DirLight {
//    vec3 direction;
  
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;
//};  
//uniform DirLight dirLight;
//struct PointLight {    
//    vec3 position;
    
//    float constant;
//    float linear;
//    float quadratic;  

//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;
//};  
//#define NR_POINT_LIGHTS 2  
//uniform PointLight pointLights[NR_POINT_LIGHTS];

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    return shadow;
}

//vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
//{
//    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    // diffuse shading
//    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
//    vec3 reflectDir = reflect(-lightDir, normal);
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
//    vec3 ambient  = 0.3  * texture(diffuseTexture, fs_in.TexCoords).rgb;
//    vec3 diffuse  = diff * lightColor;
//    vec3 specular =  spec * lightColor;
//    return (ambient + diffuse + specular);
//} 

//vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
//{
//    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    // diffuse shading
//    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
//    vec3 reflectDir = reflect(-lightDir, normal);
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
//    float distance    = length(light.position - fragPos);
//    float attenuation = 1.0 / (light.constant + light.linear * distance + 
//  			     light.quadratic * (distance * distance));    
    // combine results
//    vec3 ambient  = 0.3  * texture(diffuseTexture, fs_in.TexCoords).rgb;
//    vec3 diffuse  =  diff * lightColor;
//    vec3 specular =  spec * lightColor;
//    ambient  *= attenuation;
//    diffuse  *= attenuation;
//    specular *= attenuation;
//    return (ambient + diffuse + specular);
//} 

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    //vec3 lightColor = vec3(0.5);
    // ambient
    vec3 ambient = 0.3 * color;

    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    
	// specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    
	// calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);


 //   vec3 norm = normalize(fs_in.Normal);
 //   vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    // phase 1: Directional lighting
 //   vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
 //   for(int i = 0; i < NR_POINT_LIGHTS; i++)
 //       result += CalcPointLight(pointLights[i], norm, fs_in.FragPos, viewDir);    
    // phase 3: Spot light
    //result += CalcSpotLight(spotLight, norm, fs_in.FragPos, viewDir);    
    
 //   FragColor = vec4(result, 1.0);
}