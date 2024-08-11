#include <iostream>                        // Standard input/output stream library
#include <vector>                          // For using the std::vector container
#include <GL/glew.h>                       // GLEW library for managing OpenGL extensions
#include <GLFW/glfw3.h>                    // GLFW library for creating windows and handling input
#include <glm/glm.hpp>                     // GLM library for handling matrices and vectors
#include <glm/gtc/matrix_transform.hpp>    // GLM utilities for matrix transformations
#include <glm/gtc/type_ptr.hpp>            // GLM utilities for converting matrices to pointer types
#include "tiny_obj_loader.h"               // For loading OBJ files

// Vertex Shader source code
const char* vertexShaderSource = R"glsl(
#version 330 core                           // Specify OpenGL version 3.3 core
layout (location = 0) in vec3 aPos;         // Input vertex attribute position at location 0
uniform mat4 transform;                     // Uniform matrix for transformations
void main() {
    gl_Position = transform * vec4(aPos, 1.0); // Apply transformation to vertex position
}
)glsl";

// Fragment Shader source code
const char* fragmentShaderSource = R"glsl(
#version 330 core                           // Specify OpenGL version 3.3 core
out vec4 FragColor;                         // Output fragment color
void main() {
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // Set output color to white
}
)glsl";

// Function declaration for processing user input
void processInput(GLFWwindow* window, glm::mat4 &transform);

int main()
{
    // Initialize the GLFW library
    glfwInit();

    // Set the OpenGL version to 3.3 and use the core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 800, "A2", NULL, NULL);
    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW to manage OpenGL extensions
    glewInit();

    // Set the viewport to cover the entire window
    glViewport(0, 0, 800, 800);

    // Compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);    // Create a vertex shader object
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Attach the shader source code
    glCompileShader(vertexShader);                              // Compile the vertex shader

    // Compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create a fragment shader object
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // Attach the shader source code
    glCompileShader(fragmentShader);                            // Compile the fragment shader

    // Link shaders to create a shader program
    GLuint shaderProgram = glCreateProgram();        // Create a shader program object
    glAttachShader(shaderProgram, vertexShader);     // Attach the vertex shader
    glAttachShader(shaderProgram, fragmentShader);   // Attach the fragment shader
    glLinkProgram(shaderProgram);                    // Link the shaders into a program

    // Delete the shader objects after linking them into the program
    glDeleteShader(vertexShader);                    // Delete the vertex shader object
    glDeleteShader(fragmentShader);                  // Delete the fragment shader object

    // Load OBJ file using tinyobjloader
    std::string inputfile = "../contingo.obj"; // Path to the .obj file
    tinyobj::attrib_t attrib; // Object to store vertex attributes
    std::vector<tinyobj::shape_t> shapes; // Vector to store shapes
    std::vector<tinyobj::material_t> materials; // Vector to store materials
    std::string warn, err; // Strings to store warnings and errors

    // Load the OBJ file
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str())) {
        std::cerr << warn << err << std::endl; // Print warnings and errors if loading fails
        return 1; // Exit the program with an error code
    }

    // Extract vertices from the loaded OBJ file
    std::vector<GLfloat> vertices; // Vector to store vertex data
    for (const auto& shape : shapes) { // Loop through each shape in the OBJ file
        for (const auto& index : shape.mesh.indices) { // Loop through each index in the shape's mesh
            // Add vertex positions to the vertices vector
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]); // x-coordinate
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]); // y-coordinate
            vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]); // z-coordinate
        }
    }

    // Generate and bind Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);            // Generate VAO to store vertex attribute configuration
    glGenBuffers(1, &VBO);                 // Generate VBO to store vertex data in GPU memory

    // Bind the VAO (recording the configuration of vertex attributes)
    glBindVertexArray(VAO);

    // Bind and set the VBO's data
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind the VBO to the GL_ARRAY_BUFFER target
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW); // Copy vertex data to the VBO

    // Define vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Describe vertex attribute layout
    glEnableVertexAttribArray(0);          // Enable the vertex attribute at location 0

    // Unbind the VBO (the VAO remains bound)
    glBindBuffer(GL_ARRAY_BUFFER, 0);     // Unbind the VBO to avoid unintended modifications

    // Initialize the transformation matrix to the identity matrix
    glm::mat4 transform = glm::mat4(1.0f); // Start with the identity matrix

    // Set the polygon mode to wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Render polygons as wireframes

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) // Continue until the window should close
    {
        // Process user input and update the transformation matrix
        processInput(window, transform);

        // Clear the color buffer with a dark grey background
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Set clear color
        glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set the transformation matrix in the shader
        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform"); // Get the location of the transform uniform
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform)); // Set the transform uniform in the shader

        // Bind VAO and draw the object
        glBindVertexArray(VAO); // Bind the VAO
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3); // Draw the vertices as triangles
        glBindVertexArray(0); // Unbind the VAO

        // Swap buffers and poll for events
        glfwSwapBuffers(window); // Swap the front and back buffers
        glfwPollEvents(); // Poll for and process events
    }

    // Clean up and delete all the objects we've created
    glDeleteVertexArrays(1, &VAO);        // Delete the VAO
    glDeleteBuffers(1, &VBO);             // Delete the VBO
    glDeleteProgram(shaderProgram);         // Delete the shader program
    glfwDestroyWindow(window);              // Destroy the window
    glfwTerminate();                        // Terminate GLFW

    return 0;
}

// Function to process user input and update the transformation matrix
void processInput(GLFWwindow* window, glm::mat4 &transform) {
    // Define movement parameters
    const float translationDistance = 0.01f; // Distance for translation
    const float rotationAngle = glm::radians(1.0f); // Angle for rotation in radians
    const float scaleFactor = 1.01f; // Scaling factor

    // Close the window when the Escape key is pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); // Set the window to close

    // Translate the object based on key inputs
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        transform = glm::translate(transform, glm::vec3(0.0f, translationDistance, 0.0f)); // Move up
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        transform = glm::translate(transform, glm::vec3(0.0f, -translationDistance, 0.0f)); // Move down
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        transform = glm::translate(transform, glm::vec3(-translationDistance, 0.0f, 0.0f)); // Move left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        transform = glm::translate(transform, glm::vec3(translationDistance, 0.0f, 0.0f)); // Move right

    // Rotate the object based on key inputs
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        transform = glm::rotate(transform, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate clockwise
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        transform = glm::rotate(transform, -rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate counterclockwise

    // Scale the object based on key inputs
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        transform = glm::scale(transform, glm::vec3(scaleFactor, scaleFactor, scaleFactor)); // Scale up
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        transform = glm::scale(transform, glm::vec3(1.0f / scaleFactor, 1.0f / scaleFactor, 1.0f / scaleFactor)); // Scale down
}
