#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform int isFog;

float computeFog()
{
 	float fogDensity = 0.03f;
	float fragmentDistance = 50;
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);
}

void main()
{
    	float fogFactor = computeFog();
    	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	color = texture(skybox, textureCoordinates);
	if(isFog == 1){
		color = fogColor * (1 - fogColor) + texture(skybox, textureCoordinates) * fogFactor;
	}
}
