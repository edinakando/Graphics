#version 400
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 texCoordinates;
out vec2 textureCoordinates;
out vec3 colour;

uniform mat4 projection;
uniform vec2 offset;

void main()
{
    float scale = 10.0f;
    textureCoordinates = texCoordinates;
    colour = vec3(0.0f, 0.0f, 1.0f);
    gl_Position = projection * vec4((vertexPosition.xy * scale) + offset, 0.0, 1.0);
}