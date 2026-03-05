#version 460 core

out vec4 FragColor;

// This uniform matches the exact name we are calling in app.cpp
uniform vec4 ourColor; 

void main()
{
    // Apply the color sent from C++
    FragColor = ourColor; 
}