#version 330 core

out vec4 out_color;

in vec2 textureCoord;
in float light;
in float fog_factor;

uniform sampler2D texture0;
uniform vec3 fog_color;
uniform float light_factor;

void main()
{
	
	out_color = vec4(texture(texture0, textureCoord));
	
	vec3 fragcolor = out_color.xyz;
	fragcolor *= light;
	fragcolor = mix(fragcolor, fog_color, fog_factor);
	fragcolor *= light_factor;
	
	if(out_color.w < 0.1f)
		discard;
	
	out_color = vec4(fragcolor, out_color.w);
}

