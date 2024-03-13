#version 330 core

in vec2 textureCoord;

out vec4 out_color;

uniform sampler2D texture_0;

void main()
{
	out_color = texture(texture_0, textureCoord);
	if(out_color.w < 0.5f)
		discard;
}


