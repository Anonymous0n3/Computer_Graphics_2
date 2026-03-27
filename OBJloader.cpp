// OBJloader.cpp
#include <string>
#include <algorithm>
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <array>
#include "OBJloader.hpp"

#define MAX_LINE_SIZE 1024

bool loadOBJ(const std::filesystem::path& filename, std::vector<Vertex>& vertices, std::vector<GLuint>& indices) {
    std::cout << "Loading model: " << filename.string() << std::endl;

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    vertices.clear();
    indices.clear();

    FILE* file = nullptr;
    fopen_s(&file, filename.string().c_str(), "r");
    if (file == NULL) {
        printf("Impossible to open the file!\n");
        return false;
    }

    while (1) {
        char lineHeader[MAX_LINE_SIZE];
        int res = fscanf_s(file, "%s", lineHeader, (unsigned)MAX_LINE_SIZE);
        if (res == EOF) {
            break;
        }

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            // Task 3a: Robust face parsing
            char lineBuf[MAX_LINE_SIZE];
            fgets(lineBuf, MAX_LINE_SIZE, file); // Get the rest of the line
            std::istringstream iss(lineBuf);
            std::string token;

            std::vector<std::array<int, 3>> faceVertices; // Stores {v, vt, vn} indices

            while (iss >> token) {
                int v = 0, vt = 0, vn = 0;
                // Parse the different format possibilities (v/vt/vn, v//vn, v/vt, v)
                if (sscanf_s(token.c_str(), "%d/%d/%d", &v, &vt, &vn) == 3) {}
                else if (sscanf_s(token.c_str(), "%d//%d", &v, &vn) == 2) { vt = 0; }
                else if (sscanf_s(token.c_str(), "%d/%d", &v, &vt) == 2) { vn = 0; }
                else if (sscanf_s(token.c_str(), "%d", &v) == 1) { vt = 0; vn = 0; }

                if (v != 0) faceVertices.push_back({ v, vt, vn });
            }

            if (faceVertices.size() < 3) continue; // Invalid face

            // Task 3a: Calculate normal if missing
            glm::vec3 calculated_normal(0.0f);
            if (faceVertices[0][2] == 0) {
                glm::vec3 p0 = temp_vertices[faceVertices[0][0] - 1];
                glm::vec3 p1 = temp_vertices[faceVertices[1][0] - 1];
                glm::vec3 p2 = temp_vertices[faceVertices[2][0] - 1];
                calculated_normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            }

            // Helper lambda to process and push a vertex
            auto addVertex = [&](const std::array<int, 3>& fv) {
                Vertex currentVertex;
                currentVertex.Position = temp_vertices[fv[0] - 1];

                // Task 3a: Fake texture coordinates if missing
                currentVertex.TexCoords = (fv[1] != 0) ? temp_uvs[fv[1] - 1] : glm::vec2(0.0f);

                // Use file normal or calculated normal
                currentVertex.Normal = (fv[2] != 0) ? temp_normals[fv[2] - 1] : calculated_normal;

                // Avoid duplicate vertices
                auto t = std::find_if(vertices.begin(), vertices.end(),
                    [&currentVertex](const Vertex& v2) { return currentVertex == v2; });

                GLuint currentIndex;
                if (t == vertices.end()) {
                    vertices.push_back(currentVertex);
                    currentIndex = vertices.size() - 1;
                }
                else {
                    currentIndex = std::distance(vertices.begin(), t);
                }
                indices.push_back(currentIndex);
                };

            // Task 3a: Split Quads into two Triangles
            // First triangle (v0, v1, v2)
            addVertex(faceVertices[0]);
            addVertex(faceVertices[1]);
            addVertex(faceVertices[2]);

            // If it's a quad, add the second triangle (v0, v2, v3)
            if (faceVertices.size() == 4) {
                addVertex(faceVertices[0]);
                addVertex(faceVertices[2]);
                addVertex(faceVertices[3]);
            }
        }
    }

    std::cout << "Model loaded: " << filename.string() << " | Vertices: " << vertices.size() << std::endl;
    fclose(file);
    return true;
}