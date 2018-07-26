#version 450 core

out vec4 _outFragColor;

in vec2 _TexCoords;

uniform sampler2D uWallTexture;

void main()
{
    _outFragColor = texture(uWallTexture, _TexCoords);
}