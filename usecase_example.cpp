#include <unordered_map>
#include "ShaderProgram.hpp"
#include "Mesh.hpp"
#include "Model.hpp"

class App {
    ...
private:

    // shared library of shaders for all models, automatic resource management 
    std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shader_library;

    // shared library of meshes for all models, automatic resource management 
    std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_library;

    // all objects of the scene addressable by name
    std::unordered_map<std::string, Model> scene; 
}

void App::init_assets(void) {
    // Notice: code massively simplified - all moved to specific classes

    // all shaders: load, compile, link, initialize params, place to library
    shader_library.emplace("simple_shader", std::make_shared<ShaderProgram>("path_to.vert","path_to.frag"));
    shader_library.emplace("rainbow", std::make_shared<ShaderProgram>("path_to.vert","rainbow.frag"));

    // mesh library: meshes, that can be shared by multiple models
    mesh_library.emplace("cube", std::make_shared<Mesh>(generateCube()));
    mesh_library.emplace("sphere_lowpoly", std::make_shared<Mesh>(generateSphere(4, 4)));
    mesh_library.emplace("sphere_highpoly", std::make_shared<Mesh>(generateSphere(8, 8)));
    
    // load mesh from .OBJ
    {
        std::filesystem::path filename = "resources/models/my_model.OBJ"; // or loaded from JSON etc...
        
        if (!std::filesystem::exists(filename)) {
            throw std::runtime_error("File does not exist: " + filename.string());
        } else {
            std::vector <Vertex> & vertices;  
            std::vector <GLuint>& indices;
            if (!loadOBJ(filename, vertices, indices)) {
                throw std::runtime_error("Loading failed: " + filename.string());
            }
            
            mesh_library.emplace("loadedFromFile", std::make_shared<Mesh>(vertices,indices,GL_TRIANGLES));
        }
    }
    
    // model: load model file, assign shader used to draw a model, put to scene
    Model my_model = Model("resources/objects/hierarchical.obj", shader_library.at("simple_shader"));
    scene.push_back("my_first_object", my_model);
    
    // reuse mesh and shader data to construct complex model
    Model m;
    m.addMesh(mesh_library.at("cube"), shader_library.at("simple_shader"));
    m.addMesh(mesh_library.at("sphere_lowpoly"), shader_library.at("simple_shader"));
    m.addMesh(mesh_library.at("loadedFromFile"), shader_library.at("rainbow"));
    scene.push_back("my_complex_object", m); 
}


App::init() {
    ...
    init();
    init_assets(); // first init OpenGL, THAN init assets: valid context MUST exist  
}   

App::run() {

    glm::vec4 my_rgba = ...

    while(!glfwWindowShouldClose(window))
    {
        auto now = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       
        ...
        
        // set shader uniforms once (if all models use same shader)
        auto current_shader = shader_library.at("simple_shader"); 
        current_shader->setUniform("my_color", my_rgba);
        
        model.at("my_complex_object").origin += glm::vec3(0.1, 0.0, 0.0); // move it slightly
        
        // draw all models in the scene
        for (auto const& model : scene) {
            model.update(now);
            model.draw();
        }
 
        ...
        // end of frame
        glfwSwapBuffers();
    }

}

App::~App() {
    // clean-up
    ...
        
    std::cout << "Bye...\n";
}