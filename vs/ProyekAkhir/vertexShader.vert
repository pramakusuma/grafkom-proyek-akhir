#version 330
layout (location = 0) in vec3 position;
//layout (location = 1) in vec2 normal;

//layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

out vec3 FragPos;
out vec3 Normal;

void main()
{
	FragPos = vec3(model * vec4(position, 1.0));
    //Normal = mat3(transpose(inverse(model))) * normal; 
	Normal = aNormal;
	gl_Position = projection * view * vec4(FragPos, 1.0);
	
	//gl_Position = projection * view * model * vec4(aPos, 1.0);
	
	
	//gl_Position = projection * view * model * vec4(position, 1.0f);

	// We swap the y-axis by substracing our coordinates from 1. This is done because most images have the top y-axis inversed with OpenGL's top y-axis.
	//TexCoord = vec2(texCoord.x, 1.0 - texCoord.y);
}