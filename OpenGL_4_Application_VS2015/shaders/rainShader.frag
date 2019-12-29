#version 400
uniform sampler2D rainDropTexture;

in vec2 textureCoordinates;
in vec3 colour;

out vec4 fcolor;

void main()
{
    fcolor = vec4(colour, 1.0);
    fcolor = texture(rainDropTexture, textureCoordinates);
}  