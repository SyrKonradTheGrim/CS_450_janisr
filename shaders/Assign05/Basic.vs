#version 430 core
// Change to 410 for macOS

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;

out vec4 vertexColor;

// Uniform model matrix
uniform mat4 modelMat;
uniform mat4 projMat;
uniform mat4 viewMat;

void main()
{		
    // Get position of vertex (object space)
    vec4 objPos = vec4(position, 1.0);

    // Transform vertex position using modelMat
    vec4 viewPos = viewMat * modelMat * objPos;
    gl_Position = projMat * viewPos; 

    // Output per-vertex color
    vertexColor = color;
}