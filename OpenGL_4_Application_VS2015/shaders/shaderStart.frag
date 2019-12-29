#version 410 core

in vec3 normal;
in vec4 fragPosEye;

out vec4 fColor;
in vec2 fragTexCoords;

//lighting
uniform	mat3 normalMatrix;

uniform	vec3 lightDir;
uniform	vec3 lightColor;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

vec3 ambient;
float ambientStrength = 1f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

float att;
float specCoeff;
vec3 normalEye;
vec3 lightDirN;

void computeLightComponents()
{		
	vec3 lightPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	//lightDirN = normalize(lightDir);
	lightDirN = normalize(lightPosEye - fragPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(lightPosEye - fragPosEye.xyz);
		
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);

	//compute distance to light
	float dist = length(lightPosEye - fragPosEye.xyz);

	//compute attenuation
	att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
}

float computeFog()
{
 	float fogDensity = 0.1f;
	float fragmentDistance = length(fragPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);
}


void main() 
{
	computeLightComponents();
	
	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	vec3 baseColor = vec3(1.0f, 0.55f, 0.0f);//orange
	
	ambient = att * ambientStrength * lightColor;
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	specular = att * specularStrength * specCoeff * lightColor;
	
	ambient *= texture(diffuseTexture, fragTexCoords);
	diffuse *= texture(diffuseTexture, fragTexCoords);
	specular *= texture(specularTexture, fragTexCoords);

	vec3 color = min((ambient + diffuse) + specular, 1.0f);
    
    	//fColor = vec4(color, 1.0f);
	fColor = fogColor * (1 - fogColor) + color * fogFactor;
}
