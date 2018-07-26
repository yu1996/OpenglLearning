#version 450 core
out vec4 _outFragColor;
in vec2 _TexCoord;
uniform sampler2D uWoodTexture;

void main()
{
    _outFragColor = texture(uWoodTexture, _TexCoord);
}