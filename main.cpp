#include <iostream> // For input and output operations
#include <vector>   // For using the std::vector container
#include <GL/glew.h> // For handling OpenGL extensions
#include <GLFW/glfw3.h> // For creating and managing windows and OpenGL contexts
#include <glm/glm.hpp> // For glm::mat4 and glm::vec3
#include <glm/gtc/matrix_transform.hpp> // For glm::translate, glm::rotate, and glm::scale
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr
#include "tiny_obj_loader.h" // For loading OBJ files

// Vertex Shader source code
const char* vertexShaderSource = R"glsl(
#version 330 core
// Specify the layout location for the vertex position attribute
layout (location = 0) in vec3 aPos;

// Define a uniform variable for the transformation matrix
uniform mat4 transform;

void main() {
    // Transform the vertex position and set it to gl_Position
    gl_Position = transform * vec4(aPos, 1.0);
}
)glsl";

// Fragment Shader source code
const char* fragmentShaderSource = R"glsl(
#version 330 core
// Specify the output color of the fragment
out vec4 FragColor;

void main() {
    // Set the color of the fragment to white
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // White color
}
)glsl";

// Function declaration for processing user input
void processInput(GLFWwindow* window, glm::mat4 &transform);

int main()
{
    // Initialize GLFW library
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl; // Print error message if initialization fails
        return -1; // Exit the program with an error code
    }

    // Set GLFW window hints for OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set major version of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set minor version of OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use the core profile

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 800, "Wireframe Renderer", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl; // Print error message if window creation fails
        glfwTerminate(); // Terminate GLFW
        return -1; // Exit the program with an error code
    }

    // Make the created window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW to handle OpenGL extensions
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl; // Print error message if GLEW initialization fails
        return -1; // Exit the program with an error code
    }

    // Set the viewport size
    glViewport(0, 0, 800, 800); // Define the viewport dimensions

    // Compile and create the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Attach shader source code
    glCompileShader(vertexShader); // Compile the vertex shader

    // Compile and create the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create a fragment shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // Attach shader source code
    glCompileShader(fragmentShader); // Compile the fragment shader

    // Create a shader program and attach shaders
    GLuint shaderProgram = glCreateProgram(); // Create a shader program
    glAttachShader(shaderProgram, vertexShader); // Attach the vertex shader to the program
    glAttachShader(shaderProgram, fragmentShader); // Attach the fragment shader to the program
    glLinkProgram(shaderProgram); // Link the shaders into the program

    // Clean up shader objects after linking
    glDeleteShader(vertexShader); // Delete the vertex shader
    glDeleteShader(fragmentShader); // Delete the fragment shader

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

    // Create and configure Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
    GLuint VAO, VBO; // Variables to store VAO and VBO IDs
    glGenVertexArrays(1, &VAO); // Generate a VAO
    glGenBuffers(1, &VBO); // Generate a VBO

    glBindVertexArray(VAO); // Bind the VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind the VBO
    // Upload vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    // Specify the vertex attribute layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Enable the vertex attribute

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO

    // Initialize transformation matrix to identity matrix
    glm::mat4 transform = glm::mat4(1.0f);

    // Set the polygon mode to wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Render polygons as wireframes

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) // Continue until the window should close
    {
        // Process user input
        processInput(window, transform);

        // Clear the color buffer with a dark grey background
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Set clear color
        glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set the transform matrix uniform in the shader
        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform"); // Get location of the uniform variable
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform)); // Set the uniform value

        // Bind VAO and draw the object
        glBindVertexArray(VAO); // Bind the VAO
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3); // Draw the vertices as triangles
        glBindVertexArray(0); // Unbind the VAO

        // Swap buffers and poll for events
        glfwSwapBuffers(window); // Swap the front and back buffers
        glfwPollEvents(); // Poll for and process events
    }

    // Cleanup resources
    glDeleteVertexArrays(1, &VAO); // Delete the VAO
    glDeleteBuffers(1, &VBO); // Delete the VBO
    glDeleteProgram(shaderProgram); // Delete the shader program
    glfwDestroyWindow(window); // Destroy the GLFW window
    glfwTerminate(); // Terminate GLFW

    return 0; // Exit the program successfully
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
