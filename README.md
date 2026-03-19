# Implementation of generic resource load

- Starting point: fully working application from last week - triangle with changing color, controlled by callback.
- Finish line: new modular program, that is able to load and display externally provided models and shaders. The refactoring should make code much cleaner, because all shader related functions and model related functions will be managed by its own class (in separate files).

See __00 resource management__ directory for implementation of class inheritance, that blocks copy constructor and assign operator (move operations ARE allowed). This is necessary for safe resource management of global resources like OpenGL.

## Task 1: Generic shader loader - directory __01 shader class__

- Current state: shader hard-coded into .cpp source file as a string
- Target: two external files (suffixes .vert for vertex shader, .frag for fragment shader; if you use different suffixes, GLSL plugin can not perform syntax check and highlighting). Implemented class, that will load both shader files files, and get shader program ready. Create functions to set uniform variables for CPU-GPU communication.

1. Copy partially implemented shader class (ShaderProgram.cpp, .hpp) into your project directory and add to project.
2. Use the lecture to implement missing parts - marked as "TODO"

## Task 2: Explore directory __02 shader examples__

- see directory description, and explore the shader functionality
- some functions will be used in following lectures

## Task 3: Simple generic model loader - directory __03 vertex-mesh-model class__

- Current state: triangle vertex data are hard-coded into source code
- Resources: in subdirectory of _04 loading assets_ you can find file __triangle.obj__ with following content:

    ```text
    g triangle
    v  0.0  0.5 0.0
    v  0.5 -0.5 0.0
    v -0.5 -0.5 0.0
    
    vt 0.0 0.0
    vt 1.0 0.0
    vt 1.0 1.0

    vn 0.0 0.0 1.0

    f 1/1/1 2/2/1 3/3/1
    ```

    this defines exactly same triangle as in previous Lab, only now it is in Wavefront Object File (see <https://en.wikipedia.org/wiki/Wavefront_.obj_file> ).
    The triangle now uses texture coordinates and normal, that you should ignore for now.

- Target: implemented class that will load .OBJ file, parse the content and create VAO, VBO, set parameters etc., so the triangle data will be stored outside the source code. Use also __EBO__ (see lectures) for indirect vertex addressing.

1. Modify your __assets.hpp__, so that vertex structure contains normal and texture coordinate.
2. Copy partially implemented classes (Mesh.hpp, Model.hpp) into your project directory and add to project.
3. Explore OBJloader.cpp and OBJloader.hpp from __04 loading assets__, that can  load OBJ file. The loader is simple and limited: it expects, that model in .OBJ file __allways contains texture coordinates and normals, and uses triangles__.
4. Use the lecture to implement missing parts - marked as "TODO". Fully setup and initialize VAO. __Use DSA.__
5. Draw the triangle.

### Task 3a (OPTIONAL): Modify and extend the functionality of OBJ loader

- Loader expects triangles. Modify it, so that if it finds Quad, it will break it in two triangles.
- Loader expects normals coordinates. Modify it, so that if no normals are found, it will calculate it: for triangle, it is

```C++
glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(point1)-glm::vec3(point0), glm::vec3(point2) - glm::vec3(point0)));
```

- Loader expects texture coordinates. Modify it, so that if no texcoords are found, it will provide fake fixed coordinate glm::vec2(0.0f)

### Task 3b (superOPTIONAL): Meshlab

Download Meshlab, load some model, try to convert it to .OBJ format. Try other functions of Meshlab, like increasing/decreasing triangle count. This can be used to implement simple LOD (Level Of Detail).
