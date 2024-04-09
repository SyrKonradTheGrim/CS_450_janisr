#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <GL/glew.h>					
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "MeshData.hpp"
#include "MeshGLData.hpp"
#include "GLSetup.hpp"
#include "Shader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

// Create very simple mesh: a quad (4 vertices, 6 indices, 2 triangles)
void createSimpleQuad(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create four corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;

	// Set positions of vertices
	// Note: glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Add to mesh's list of vertices
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	
	// Add indices for two triangles
	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);
}

void createSimplePentagon(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create four corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;
	Vertex top;

	// Set positions of vertices
	// Note: glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);
	top.position = glm::vec3(0.0, 1.0, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	top.color = glm::vec4(1.0, 1.0, 0.0, 0.0);

	// Add to mesh's list of vertices
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	m.vertices.push_back(top);
	
	// Add indices for two triangles
	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);

	m.indices.push_back(1);
	m.indices.push_back(0);
	m.indices.push_back(4);
}

void extractMeshData(aiMesh *mesh, Mesh &m) {
	// Clear out the Mesh's vertices and indices
	m.vertices.clear();
	m.indices.clear();

	// Loop through all vertices in the aiMesh
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        // Convert aiVector3D to glm::vec3 for position
        aiVector3D aiPos = mesh->mVertices[i];
        vertex.position = glm::vec3(aiPos.x, aiPos.y, aiPos.z);

        // Set the color of the Vertex (using a simple scheme here, but you can get creative)
        float colorComponent = static_cast<float>(i) / mesh->mNumVertices; // Simple gradient effect
        vertex.color = glm::vec4(colorComponent, 1.0f - colorComponent, 0.5f, 1.0f); // RGBA, A always 1.0f

        // Add the Vertex to the Mesh's vertices list
        m.vertices.push_back(vertex);
    }

	// Loop through all faces in the aiMesh
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        // Loop through the number of indices for this face
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            // Add each index for the face to the Mesh's list of indices
            m.indices.push_back(face.mIndices[j]);
        }
	}


}

// Main 
int main(int argc, char **argv) {
    // Are we in debugging mode?
    bool DEBUG_MODE = true;

    // GLFW setup
    // Switch to 4.1 if necessary for macOS
    GLFWwindow* window = setupGLFW("Assign03: janisr", 4, 3, 800, 800, DEBUG_MODE);

    // GLEW setup
    setupGLEW(window);

    // Check OpenGL version
    checkOpenGLVersion();

    // Set up debugging (if requested)
    if(DEBUG_MODE) checkAndSetupOpenGLDebugging();

    // Set the background color to a shade of blue
    glClearColor(1.0f, 1.0f, 0.8f, 1.0f);


    // Command line argument handling for model path
    string modelPath = "sampleModels/sphere.obj";
    if (argc >= 2) {
        modelPath = argv[1];
    }

    // Load the model using Assimp
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelPath, 
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cerr << "ERROR: Failed to load model: " << modelPath << endl;
        cleanupGLFW(window);
        return -1;
    }
    
	// Create and load shaders
	//GLuint programID = 0;
	try {		
		// Load vertex shader code and fragment shader code
		string vertexCode = readFileToString("./shaders/Assign03/Basic.vs");
		string fragCode = readFileToString("./shaders/Assign03/Basic.fs");

		// Print out shader code, just to check
		if(DEBUG_MODE) printShaderCode(vertexCode, fragCode);

		// Create shader program from code
		programID = initShaderProgramFromSource(vertexCode, fragCode);
	}
	catch (exception e) {		
		// Close program
		cleanupGLFW(window);
		exit(EXIT_FAILURE);
	}


    // Create MeshGL vector to store all meshes
    vector<MeshGL> meshGLVector;

    // Process each mesh in the loaded model
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        Mesh m;
        extractMeshData(scene->mMeshes[i], m); // Extract mesh data from Assimp's mesh

        MeshGL mgl;
        createMeshGL(m, mgl); // Convert mesh data to GPU-ready format

        meshGLVector.push_back(mgl); // Store MeshGL for drawing
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        // Set viewport size
        int fwidth, fheight;
        glfwGetFramebufferSize(window, &fwidth, &fheight);
        glViewport(0, 0, fwidth, fheight);

        // Clear the framebuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        glUseProgram(programID);

        // Draw each MeshGL object
        for (auto& mgl : meshGLVector) {
            drawMesh(mgl); // Draw each mesh
        }

        // Swap buffers and poll for window events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Sleep for 15 ms
        // this_thread::sleep_for(chrono::milliseconds(15));
    }

    // Clean up all MeshGL objects
    for (auto& mgl : meshGLVector) {
        cleanupMesh(mgl);
    }

    // Clean up GLFW and OpenGL resources
    glUseProgram(0);
    glDeleteProgram(programID);
    cleanupGLFW(window);

    return 0;
}