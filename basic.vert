#version 460 core

// Input vertex attributes
layout(location = 0) in vec3 aPos;   // position: MUST exist
layout(location = 1) in vec3 aColor; // color (or normal/uv depending on your Mesh class)

// Uniform matrices for transformations (Task 2)
// Defaulting to identity matrix mat4(1.0) makes it safe to build step-by-step
uniform mat4 model = mat4(1.0);
uniform mat4 view = mat4(1.0);
uniform mat4 projection = mat4(1.0);

out vec3 color; // optional output attribute

void main()
{
    // The order of matrix multiplication is crucial here!
    // In GLSL, matrix multiplication happens from right to left:
    // 1. model * position (moves object into world space)
    // 2. view * result (moves world relative to camera)
    // 3. projection * result (applies perspective/clipping)
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    
    color = aColor; // copy color to output
}