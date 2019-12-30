#version 400

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 textCoord;

out vec3 colour;
out vec2 passTexture; 

uniform mat4 view;
uniform mat4 projection;
uniform vec3 offset;

void main() {
    colour = vec3(0.0f, 0.0f, 1.0f);
    gl_Position = projection  * view * vec4(vertexPosition + offset, 1.0);
    passTexture = textCoord;
}
