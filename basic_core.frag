#version 460 core

uniform vec4 uniform_Color = vec4(1.0);

out vec4 FragColor;

void main() {
	FragColor = uniform_Color;
}
