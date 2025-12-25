#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    //initialize GLFW
    glfwInit();

    // gives hint to glfw about the context version (opengl version) and profile (core/compatibility)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create a window
    // Parameters: width, height, title, monitor (for fullscreen, nullptr for windowed), share (for sharing resources, nullptr if not needed)
    GLFWwindow* window = glfwCreateWindow(800, 800, "OpenGL Window", nullptr, nullptr);

    // check if window creation was successful
    if(window == nullptr){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // make the context of the window current on the calling thread
    // current means that any subsequent OpenGL calls will affect this window's context
    glfwMakeContextCurrent(window);

    // load OpenGL function pointers using glad
    gladLoadGL();

    // set the canvas width for openGL rendering
    // in this case its from x = 0 to x = 800 and y = 0 to y = 800
    glViewport(0, 0, 800, 800); // set the viewport size

    // set the clear color for the window
    // this color will be used whenever the color buffer is cleared
    // this is the back buffer color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // set the clear color

    glClear(GL_COLOR_BUFFER_BIT); // clear the color buffer with the clear color

    glfwSwapBuffers(window); // swap the front and back buffers

    // loop until the user closes the window
    while(!glfwWindowShouldClose(window)){

        glfwPollEvents(); // process events like keyboard and mouse input
    }

    // delete window and its context before terminating GLFW
    glfwDestroyWindow(window);

    // make the context of the window current on the calling thread
    glfwTerminate();
    return 0;
}
