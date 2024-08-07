cmake_minimum_required(VERSION 3.28)
project(A3)

set(CMAKE_CXX_STANDARD 17)

# Define the executable
add_executable(A3 main.cpp)

# Specify the include directories
include_directories(/opt/homebrew/Cellar/glfw/3.4/include)
include_directories(/opt/homebrew/Cellar/glm/1.0.1/include/glm)
include_directories(/opt/homebrew/Cellar/glew/2.2.0_1/include/GL)
include_directories(/Users/tuananhpham/tinyobjloader) # Include tinyobjloader

# Find and include the GLEW package
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# Manually set GLEW library path if not found
if(NOT GLEW_LIBRARIES)
    set(GLEW_LIBRARIES /opt/homebrew/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.0.dylib)
endif()

# Find and include the GLFW package
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

# Find and include the OpenGL framework
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# Link the libraries to the target executable
target_link_libraries(A3
        ${GLEW_LIBRARIES}
        glfw
        ${OPENGL_LIBRARIES}
        /Users/tuananhpham/tinyobjloader/build/libtinyobjloader.a) # Link tinyobjloader
