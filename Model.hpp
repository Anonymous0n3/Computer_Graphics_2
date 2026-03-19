#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory> 
#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp> // <-- This is the missing piece!

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"

class Model {
public:
    // origin point of whole model
  

    // mesh related data
    struct mesh_package {
        std::shared_ptr<Mesh> mesh;         // geometry & topology, vertex attributes
        std::shared_ptr<ShaderProgram> shader;     // which shader to use to draw this part of the model

        glm::vec3 origin;                   // mesh origin relative to origin of the whole model
        glm::vec3 eulerAngles;              // mesh rotation relative to orientation of the whole model
        glm::vec3 scale;                    // mesh scale relative to scale of the whole model
    };
    std::vector<mesh_package> meshes;
    
    Model() = default;
    Model(const std::filesystem::path & filename, std::shared_ptr<ShaderProgram> shader) {
        // Load mesh (all meshes) of the model, (in the future: load material of each mesh, load textures...)
        // notice: you can load multiple meshes and place them to proper positions, 
        //            multiple textures (with reusing) etc. to construct single complicated Model   
        //
        // This can be done by extending OBJ file parser (OBJ can load hierarchical models),
        // or by your own JSON model specification (or keep it simple and set a rule: 1model=1mesh ...) 
        //
    }

    void addMesh(std::shared_ptr<Mesh> mesh,
                 std::shared_ptr<ShaderProgram> shader, 
                 glm::vec3 origin = glm::vec3(0.0f),      // dafault value
                 glm::vec3 eulerAngles = glm::vec3(0.0f), // dafault value
                 glm::vec3 scale = glm::vec3(1.0f)       // dafault value
                 ) {
        meshes.emplace_back(mesh,shader,origin,eulerAngles,scale);
    }

    // update based on running time
    void update(const float delta_t) {
        // change internal state of the model (positions of meshes, size, etc.) 
        // note: this allows dynamic behaviour - it can be modified to 
        //       use lambda funtion, call scripting language, etc. 
    }
    
    void draw() {
        for (auto const& mesh_pkg : meshes) {
            mesh_pkg.shader->use(); // Activate the shader

            // Calculate the specific mesh's matrix and combine it with the model's overall matrix
            glm::mat4 mesh_model_matrix = createMM(mesh_pkg.origin, mesh_pkg.eulerAngles, mesh_pkg.scale);

            // Send the matrix to the shader uniform "uM_m"
            mesh_pkg.shader->setUniform("uM_m", mesh_model_matrix * local_model_matrix);

            mesh_pkg.mesh->draw();   // Finally, draw it!
        }
    }

    void setPosition(const glm::vec3& new_position) {
        pivot_position = new_position;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    void setEulerAngles(const glm::vec3& new_eulerAngles) {
        eulerAngles = new_eulerAngles;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    void setScale(const glm::vec3& new_scale) {
        scale = new_scale;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    // Relative movement (adding to current state)
    void translate(const glm::vec3& offset) {
        pivot_position += offset;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }

    void rotate(const glm::vec3& pitch_yaw_roll_offs) {
        eulerAngles += pitch_yaw_roll_offs;
        local_model_matrix = createMM(pivot_position, eulerAngles, scale);
    }
private:
    glm::vec3 pivot_position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 eulerAngles{ 0.0f, 0.0f, 0.0f };    // pitch, yaw, roll (in degrees)
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

    // Cache for the final matrix
    glm::mat4 local_model_matrix{ 1.0f };

    // Helper to keep angles within 0-360 degrees
    float wrapAngle(float angle) {
        angle = std::fmod(angle, 360.0f);
        if (angle < 0.0f) {
            angle += 360.0f;
        }
        return angle;
    }

    // Calculates the Model Matrix (Translation * Rotation * Scale)
    glm::mat4 createMM(const glm::vec3& origin, const glm::vec3& eAng, const glm::vec3& scl) {
        glm::vec3 eA{ wrapAngle(eAng.x), wrapAngle(eAng.y), wrapAngle(eAng.z) };

        glm::mat4 t = glm::translate(glm::mat4(1.0f), origin);
        glm::mat4 rotm = glm::yawPitchRoll(glm::radians(eA.y), glm::radians(eA.x), glm::radians(eA.z));
        glm::mat4 s = glm::scale(glm::mat4(1.0f), scl);
        // Note: The order of multiplication matters in matrix math!
        return s * rotm * t;
    }
};

