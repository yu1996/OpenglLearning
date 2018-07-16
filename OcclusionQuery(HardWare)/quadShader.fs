#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uWindowTexture;

void main()
{
    FragColor = texture(uWindowTexture, TexCoords);
    //FragColor = vec4(1.0, 0.0, 0.0, 0.5);
}