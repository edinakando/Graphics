#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;
in vec2 fragTexCoords;
in mat4 fView;

uniform	mat3 normalMatrix;
uniform	vec3 lightDir;
uniform mat3 lightDirMatrix;
uniform	vec3 lightColor;
uniform vec3 pointLightColor;
uniform vec3 pointLightPosition1;
uniform vec3 pointLightPosition2;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform int isFog;
uniform int isNight;

out vec4 fColor;

vec3 ambient;
float ambientStrength = 0.3f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 64.0f;

vec3 ambientPoint;
vec3 diffusePoint;
vec3 specularPoint;
float constant = 1.0f;
float linear = 0.35f;
float quadratic = 0.44f;

float att;
float specCoeff;
vec3 normalEye;
vec3 lightDirN;

vec3 computeDirectLightComponents()
{	
	vec3 lightColor;
	if(isNight == 0)
		lightColor = vec3(1.0f, 1.0f, 1.0f);
	else lightColor = vec3(0.0f, 0.0f, 0.0f);
		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	lightDirN = normalize(lightDirMatrix * lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
		
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);

	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
	
	return (ambient + diffuse + specular);
}

vec3 computePointLightComponents(vec4 lightPos){
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin

	vec3 normalEye = normalize(normalMatrix * normal);
    vec3 lightDir = normalize(lightPos.xyz - fragPosEye.xyz);
	vec3 viewDirN = normalize(cameraPosEye- fragPosEye.xyz);
	vec3 halfVector = normalize(lightDir + viewDirN);

	float dist = length(lightPos - fragPosEye.xyz);
	att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);

	ambientPoint = att * ambientStrength * pointLightColor;
	diffusePoint = att * max(dot(normalEye, lightDir), 0.0f) * pointLightColor;
	specularPoint = att * specularStrength * specCoeff * pointLightColor;
	
	ambientPoint *= vec3(texture(diffuseTexture, fragTexCoords));
	diffusePoint *= vec3(texture(diffuseTexture, fragTexCoords));
	specularPoint *= vec3(texture(specularTexture, fragTexCoords));
	
	return (ambientPoint + diffusePoint + specularPoint);
}

float computeShadow()
{	
    // perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;

    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;

    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    
    // Check whether current frag pos is in shadow
    //float bias = 0.005f;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0f : 0.0f;

    return shadow;	
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
	vec3 directionalLight = computeDirectLightComponents();
	vec4 pointLight1 = fView * vec4(pointLightPosition1, 1.0f);
	vec4 pointLight2 = fView * vec4(pointLightPosition2, 1.0f);

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	float shadow = computeShadow();
	
	ambient *= vec3(texture(diffuseTexture, fragTexCoords));
	diffuse *= vec3(texture(diffuseTexture, fragTexCoords));
	specular *= vec3(texture(specularTexture, fragTexCoords));
	
	//modulate with shadow
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    
	if(isNight == 1){
		color += computePointLightComponents(pointLight1);
		color += computePointLightComponents(pointLight2);
	}
		
	if(isFog == 1){
		fColor = fogColor * (1 - fogColor) + color * fogFactor;
	}
        else {
		fColor = vec4(color, 1.0f);
	}
}
