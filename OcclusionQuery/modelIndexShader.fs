#version 450 core
uniform float uModelIndex;

out float _outFragColor;

void main()
{
    _outFragColor = uModelIndex;
}