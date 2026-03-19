#version 460 core

// Your vertex attributes (matching your Mesh class)
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

// The Transformation Matrices
uniform mat4 uM_m; // Model Matrix
uniform mat4 uV_m; // View Matrix
uniform mat4 uP_m; // Projection Matrix

void main() {
    // Note: Matrix multiplication in GLSL is read right-to-left!
    // 1. We start with the local vertex position.
    // 2. Multiply by Model matrix (moves it into the world).
    // 3. Multiply by View matrix (moves it relative to the camera).
    // 4. Multiply by Projection matrix (applies perspective/zoom).
    
    gl_Position = uP_m * uV_m * uM_m * vec4(position, 1.0);
}