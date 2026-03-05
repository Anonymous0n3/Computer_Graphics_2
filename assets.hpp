#pragma once

#include <glm/glm.hpp> 

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    // We need to add this equality operator so the std::find_if 
    // function in your OBJloader.cpp knows how to compare two vertices!
    bool operator==(const Vertex& other) const {
        return Position == other.Position &&
            Normal == other.Normal &&
            TexCoords == other.TexCoords;
    }
};

