#version 330 core

out vec4 out_color;

uniform float light_factor;
uniform vec2 screen;
uniform vec3 color_sky;
uniform vec3 color_horizon;
uniform vec3 view_direction;

void main()
{
	float cos_theta = dot(view_direction, vec3(0.0f, 1.0f, 0.0f));
	
	float disp_pos = gl_FragCoord.y / screen.y - 0.5f;
	//disp_pos *= 60.0f;
	
	vec3 finalColor = light_factor * mix(color_horizon, color_sky, clamp((cos_theta + disp_pos - 0.2f) * 3.0f, 0.0f, 1.0f));
	
	out_color = vec4(finalColor, 1.0f);
}

