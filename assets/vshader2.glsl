#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aCoord;

out vec2 textureCoord;

uniform vec4 atlas_values;
uniform vec4 trans_values;

void main() {
	textureCoord = aCoord * vec2(atlas_values.z, atlas_values.w) + vec2(atlas_values.x, atlas_values.y);
	gl_Position = vec4(aPos.x * trans_values.x + trans_values.z, aPos.y * trans_values.y + trans_values.w, 0.0, 1.0);
}

