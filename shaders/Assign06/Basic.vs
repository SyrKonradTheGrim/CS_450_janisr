#version 430 core
// Change to 410 for macOS

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location=2) in vec3 normal;

uniform mat4 modelMat;
uniform mat4 projMat;
uniform mat4 viewMat;

uniform mat3 normMat;

out vec4 vertexColor;
out vec4 interPos;
out vec3 interNormal;

void main()
{		
    // Get position of vertex (object space)
    vec4 objPos = vec4(position, 1.0);

    // Transform vertex position using modelMat
    vec4 viewPos = viewMat * modelMat * objPos;
    interPos = projMat * viewPos;
    gl_Position = projMat * viewPos; 

    interNormal = normMat * normal;

    // Output per-vertex color
    vertexColor = color;
}