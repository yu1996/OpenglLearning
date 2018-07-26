#version 430 core

out vec4 _outFragColor;

flat in vec3 VertexColor;

void main()
{
	_outFragColor = vec4(VertexColor, 1.f);
}