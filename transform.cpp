#include "camera.hpp"

App {
private:
    int width, height;
    float fov;
    glm::mat4 projection_matrix;

    
    // camera related 
    Camera camera;
    // remember last cursor position, move relative to that in the next frame
    double cursorLastX{ 0 };
    double cursorLastY{ 0 };
}
//--------------------------------------------------------------------------

//init()
//update_projection_matrix();

//callbacks()

void App::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto app = static_cast<App*>(glfwGetWindowUserPointer(window));

    app->camera.ProcessMouseMovement(xpos - app->cursorLastX, (ypos - app->cursorLastY) * -1.0);
    app->cursorLastX = xpos;
    app->cursorLastY = ypos;
}

//=========================================================================

int App::run() {
    //...
    
    //add:
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // disable cursor, so that it can not leave window, and we can process movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // get first position of mouse cursor
    glfwGetCursorPos(window, &cursorLastX, &cursorLastY);

    
    update_projection_matrix();
    glViewport(0, 0, width, height);
    
    camera.position = glm::vec3(0, 0, 1000);
    double last_frame_time = glfwGetTime();

    // we have only one shader...
    shader.activate();
     
    while (!glfwWindowShouldClose(globals.window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        //########## react to user  ##########
        double delta_t = // render time of the last frame 
        camera.processInput(window, delta_t); // process keys etc.
        
        //########## create and set View Matrix according to camera settings  ##########
		shader.setUniform("uV_m", camera.GetViewMatrix());		
		shader.setUniform("uP_m", projection_matrix);		

        // draw all
        for (auto& [name, model] : scene) {
           model.draw();
        }

        // Swap front and back buffers
        glfwSwapBuffers(globals.window);

        // Poll for and process events
        glfwPollEvents();
    }
}

