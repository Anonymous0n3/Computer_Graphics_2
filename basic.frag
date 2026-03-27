#version 460 core
out vec4 FragColor;

in vec2 TexCoords; // Přijímáme z Vertex Shaderu

uniform vec4 ourColor; 
uniform sampler2D tex0; // Naše textura

void main()
{
    // Smícháme barvu z textury s naší uniform barvou
    FragColor = texture(tex0, TexCoords) * ourColor; 
}