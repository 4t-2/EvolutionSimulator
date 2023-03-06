#version 330 core

in vec2 UVcoord;
in vec4 fragColor;

out vec4 color;

uniform sampler2D textureSampler;

void main()
{
    color = texture(textureSampler, UVcoord) * fragColor;
}
