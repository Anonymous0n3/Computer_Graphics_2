//global variables


class App {
    //...
protected:
    // projection related variables    
    int width{0}, height{0};
    float fov = 60.0f;
    // store projection matrix here, update only on callbacks
    glm::mat4 projection_matrix = glm::identity<glm::mat4>();

    
    
    // all objects of the scene
    std::unordered_map<std::string, Model> scene;    
}

//--------------------------------------------------------------------------

App::init() {
    //...
    glfwSetFramebufferSizeCallback(window, fbsize_callback);    // On GL framebuffer resize callback.
    glfwSetScrollCallback(window, scroll_callback);             // On mouse wheel.
    //...

    // Load models...
}

void App::scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
    // get App instance
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    this_inst->fov += 10*yoffset; // yoffset is mostly +1 or -1; one degree difference in fov is not visible
    this_inst->fov = std::clamp(this_inst->fov, 20.0f, 170.0f); // limit FOV to reasonable values...
    
    this_inst->update_projection_matrix(); 
}

void App::fbsize_callback(GLFWwindow* window, int width, int height)
{
    auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    this_inst->width = width;
    this_inst->height = height;

    // set viewport
    glViewport(0, 0, width, height);
    //now your canvas has [0,0] in bottom left corner, and its size is [width x height] 

    this_inst->update_projection_matrix(); 
};

//--------------------------------------------------------------------------

void App::update_projection_matrix(void)
{
    if (height < 1)
        height = 1;   // avoid division by 0

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
        ratio,               // Aspect Ratio. Depends on the size of your window.
        0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
        20000.0f             // Far clipping plane. Keep as little as possible.
    );
}

//=========================================================================

void App::run() {
    //...
    
    // first update = manual (no event for update arrived yet)
    update_projection_matrix();
    glViewport(0, 0, width, height);
     
    while (!glfwWindowShouldClose(globals.window))
    {
        // Clear OpenGL canvas, both color buffer and Z-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        //set transformations

        //set View matrix = set CAMERA
        glm::mat4 v_m = glm::lookAt(
                            glm::vec3(0, 0, 1000), // position of camera
                            glm::vec3(0, 0, 0),    // where to look
                            glm::vec3(0, 1, 0)     // up direction
                        );
        
        // set uniforms for shader - common for all objects (do not set for each object individually, they use same shader anyway)
        shader.setUniform("uV_m", v_m);     
        shader.setUniform("uP_m", projection_matrix);       

       
    //#######              DRAW             ###############
    // option 1: draw object, no changes in position, rotation, scale, etc.
        scene.at("model_of_something").draw();
    //#####################################################
    // option 2: compute and save new position, simple transformation
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            scene.at("model_of_something").rotate(glm::vec3(0.0f, static_cast<float>(360.0f*glfwGetTime()), 0.0f));
        } else
            scene.at("model_of_something").draw();
    //#####################################################
    // option 3: - for complex transformations: combine multiple transformations to model matrix  
    //           - prepare model transformation locally, stack it with internal position
    //           - see Model-extended.cpp for draw(glm::mat4 const & model_matrix)
        glm::mat4 m_m = glm::identity<glm::mat4>();
        m_m = glm::rotate(m_m, glm::radians(static_cast<float>(360.0f*glfwGetTime())), glm::vec3(0.0f, 0.1f, 0.0f));
        m_m = glm::scale(m_m, glm::vec3(1.0f + glm::sin(glfwGetTime())));
        scene.at("model_of_something").setModelMatrix(m_m);
        scene.at("model_of_something").draw();
    //#####################################################
    // option 4: (for self-modifying object): derive new class - rotatingModel, modify update() method, so that it modifies member variable rotation itself
    //                                        (or define lambda function, or use scripting(see last lecture))
        scene.at("model_of_something").update(delta_t);
        scene.at("model_of_something").draw();


        
        // Swap front and back buffers
        glfwSwapBuffers(globals.window);

        // Poll for and process events
        glfwPollEvents();
    }
}
