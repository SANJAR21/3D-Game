#version 330 core

in vec2 toTexCoord;
out vec4 FragColor;

// texture sampler
uniform sampler2D texture_diffuse;

void main()
{
    FragColor = texture(texture_diffuse, toTexCoord);
}
