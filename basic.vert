#version 460 core
layout(location = 0) in vec3 aPos;   
layout(location = 1) in vec3 aNormal; // Tvůj Mesh používá normals na lokaci 1
layout(location = 2) in vec2 aTexCoords; // Task 4: UV souřadnice na lokaci 2

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords; // Pošleme do Fragment Shaderu

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    TexCoords = aTexCoords;
}