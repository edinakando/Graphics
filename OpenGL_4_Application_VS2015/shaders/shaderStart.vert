#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 normal;
out vec4 fragPosEye;
out vec4 fragPosLightSpace;
out vec2 fragTexCoords;
out mat4 fView;
out vec4 worldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceTrMatrix;

void main() 
{
	//compute eye space coordinates
	worldPos = model * vec4(vPosition, 1.0f);
	fragPosEye = view * model * vec4(vPosition, 1.0f);
	normal = vNormal;
	fragTexCoords = vTexCoords;
	fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
	fView = view;
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
}
