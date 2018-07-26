#version 450 core
out vec4 _outFragColor;
in vec2 _TexCoord;
uniform sampler2D uBoxTexture;

void main()
{
    _outFragColor = texture(uBoxTexture, _TexCoord);
}