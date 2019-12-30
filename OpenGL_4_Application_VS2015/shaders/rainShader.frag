#version 400

in vec3 colour;
in vec2 passTexture; 

out vec4 fragmentColour;
uniform sampler2D rainDropTexture;

void main() {
    fragmentColour = vec4(colour, 1.0);
    fragmentColour = texture(rainDropTexture, passTexture); 
}