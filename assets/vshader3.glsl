#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aCoord;
layout (location = 2) in float aLight;

out vec2 textureCoord;
out float light;
out float fog_factor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;

uniform vec2 coordFactors;
uniform vec2 coordOffsets;

uniform float fogDensity;

void main() {
	light = aLight;
	textureCoord = vec2(aCoord.x, aCoord.y) * coordFactors + coordOffsets;
	
	vec4 posView = view * transform *  vec4(aPos, 1.0);
	float fragDist = length(posView.xyz);
	float curved = posView.y - 0.0008f * fragDist * fragDist;
	posView.y = curved;
	
	fragDist = clamp(fragDist - 24.0f, 0.0f, 999.0f), 
	fog_factor = clamp(fragDist * fogDensity, 0.0f, 1.0f);
	
	gl_Position = projection * posView;
}

