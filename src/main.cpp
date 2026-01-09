#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

/*
    attributes:
    - position (attribute location 0)
    - color (attribute location 1)
    - texture coordinates (attribute location 2)
*/

int main() {
    //initialize GLFW
    glfwInit();

    // gives hint to glfw about the context version (opengl version) and profile (core/compatibility)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // vertex data for a triangle
    GLfloat vertices[] = {
        -0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // lower left corner
         0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // lower right corner
         0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f  // upper corner
    };

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

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // create a vertex shader object
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr); // attach the shader source code to the shader object
    glCompileShader(vertexShader); // compile the vertex shader

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // create a fragment shader object
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr); // attach the shader source code to the shader object
    glCompileShader(fragmentShader); // compile the fragment shader

    GLuint shaderProgram = glCreateProgram(); // create a shader program object
    glAttachShader(shaderProgram, vertexShader); // attach the vertex shader to the shader program
    glAttachShader(shaderProgram, fragmentShader); // attach the fragment shader to the shader program

    glLinkProgram(shaderProgram); // link the shader program

    glDeleteShader(vertexShader); // delete the vertex shader as it's no longer needed
    glDeleteShader(fragmentShader); // delete the fragment shader as it's no longer needed


    // VBO stores vertex data in GPU memory
    // VAO stores the configuration of vertex attributes or basically how the vertex data is interpreted
    GLuint VAO, VBO; // declare variables for Vertex Array Object and Vertex Buffer Object

    glGenVertexArrays(1, &VAO); // generate a VAO ID
    glGenBuffers(1, &VBO); // generate a buffer ID for the VBO

    glBindVertexArray(VAO); // bind the VAO as the current vertex array object

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // bind the VBO as the current array buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy the vertex data into the VBO

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // define the vertex attribute layout
    glEnableVertexAttribArray(0); // enable the vertex attribute at location 0, location 0 corresponds to 'aPos' in the vertex shader or the position attribute

    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind the VBO
    glBindVertexArray(0); // unbind the VAO

    // set the clear color for the window
    // this color will be used whenever the color buffer is cleared
    // this is the back buffer color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // set the clear color

    glClear(GL_COLOR_BUFFER_BIT); // clear the color buffer with the clear color

    glfwSwapBuffers(window); // swap the front and back buffers

    // loop until the user closes the window
    while(!glfwWindowShouldClose(window)){
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram); // use the shader program
        glBindVertexArray(VAO); // bind the VAO
        glDrawArrays(GL_TRIANGLES, 0, 3); // draw the triangle
        glfwSwapBuffers(window); // swap the front and back buffers

        glfwPollEvents(); // process events like keyboard and mouse input
    }

    glDeleteVertexArrays(1, &VAO); // delete the VAO
    glDeleteBuffers(1, &VBO); // delete the VBO
    glDeleteProgram(shaderProgram); // delete the shader program

    // delete window and its context before terminating GLFW
    glfwDestroyWindow(window);

    // make the context of the window current on the calling thread
    glfwTerminate();
    return 0;
}
