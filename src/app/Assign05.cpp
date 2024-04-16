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
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Utility.hpp"

#define GLM_ENABLE_EXPERIMENTAL

using namespace std;

float rotAngle = 0.0f;

// Globals
glm::vec3 eye(0.0f, 0.0f, 1.0f); // Default camera position
glm::vec3 lookAt(0.0f, 0.0f, 0.0f); // Default look-at point
glm::vec2 mousePos(0.0f, 0.0f); // Initial mouse position

glm::mat4 makeLocalRotate(glm::vec3 offset, glm::vec3 axis, float angle) {
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -offset);
    glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    glm::mat4 translateForward = glm::translate(glm::mat4(1.0f), offset);
    return translateForward * rotate * translateBack;
}

glm::mat4 makeRotateZ(glm::vec3 offset) {
    // Convert rotAngle to radians
    float radians = glm::radians(rotAngle);

    // Generate transformation matrices
    glm::mat4 translate1 = glm::translate(glm::mat4(1.0f), -offset);
    glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translate2 = glm::translate(glm::mat4(1.0f), offset);

    // Form composite transformation
    return translate2 * rotate * translate1;
}

static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    glm::vec2 relMouse = -(glm::vec2(xpos, ypos) - mousePos);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width > 0 && height > 0) {
        float scaleX = relMouse.x / static_cast<float>(width);
        float scaleY = relMouse.y / static_cast<float>(height);

        glm::vec3 cameraDir = glm::normalize(lookAt - eye);
        glm::vec3 globalYAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 localXAxis = glm::normalize(glm::cross(cameraDir, globalYAxis));

        glm::mat4 rotationX = makeLocalRotate(eye, localXAxis, 30.0f * scaleY);
        glm::mat4 rotationY = makeLocalRotate(eye, globalYAxis, 30.0f * scaleX);

        lookAt = glm::vec3(rotationX * glm::vec4(lookAt, 1.0));
        lookAt = glm::vec3(rotationY * glm::vec4(lookAt, 1.0));
    }
    mousePos = glm::vec2(xpos, ypos);
}

void renderScene(vector<MeshGL> &allMeshes, aiNode *node, glm::mat4 parentMat, GLint modelMatLoc, int level) {
    // Get transformation for the current node
    glm::mat4 nodeT;
    aiMatToGLM4(node->mTransformation, nodeT);

    // Compute current model matrix
    glm::mat4 modelMat = parentMat * nodeT;

    // Get location of current node
    glm::vec3 pos = glm::vec3(modelMat[3]);

    // Proper local Z rotation
    glm::mat4 R = makeRotateZ(pos);

    // Generate temporary model matrix
    glm::mat4 tmpModel = R * modelMat;

    // Pass tmpModel as model matrix
    glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(tmpModel));

    // Render each mesh in the node
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        int index = node->mMeshes[i];
        drawMesh(allMeshes.at(index));
    }

    // Render each child node recursively
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        renderScene(allMeshes, node->mChildren[i], modelMat, modelMatLoc, level + 1);
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float speed = 0.1f;
        glm::vec3 cameraDir = glm::normalize(lookAt - eye);
        glm::vec3 globalYAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 localXAxis = glm::normalize(glm::cross(cameraDir, globalYAxis));
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_J:
                rotAngle += 1.0f;
                break;
            case GLFW_KEY_K:
                rotAngle -= 1.0f;
                break;
            default:
                break;
            case GLFW_KEY_W:
                eye += speed * cameraDir;
                lookAt += speed * cameraDir;
                break;
            case GLFW_KEY_S:
                eye -= speed * cameraDir;
                lookAt -= speed * cameraDir;
                break;
            case GLFW_KEY_D:
                eye += speed * localXAxis;
                lookAt += speed * localXAxis;
                break;
            case GLFW_KEY_A:
                eye -= speed * localXAxis;
                lookAt -= speed * localXAxis;
                break;
        }
    }
}

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

    m.indices.push_back(0);
	m.indices.push_back(1);
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
    GLFWwindow* window = setupGLFW("Assign05: janisr", 4, 3, 800, 800, DEBUG_MODE);

    // GLEW setup
    setupGLEW(window);

    // Get the initial position of the mouse
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    mousePos = glm::vec2(mx, my);

    // Set cursor motion
    glfwSetCursorPosCallback(window, mouse_position_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Check OpenGL version
    checkOpenGLVersion();

    // Set up debugging (if requested)
    if(DEBUG_MODE) checkAndSetupOpenGLDebugging();

    // Set the background color to a shade of blue
    glClearColor(1.0f, 1.0f, 0.8f, 1.0f);

    // Create and load shaders
	GLuint programID = 0;
	try {		
		// Load vertex shader code and fragment shader code
		string vertexCode = readFileToString("./shaders/Assign05/Basic.vs");
		string fragCode = readFileToString("./shaders/Assign05/Basic.fs");

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

    // Command line argument handling for model path
    string modelPath = "sampleModels/bunnyteatime.glb";
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

    // Use shader program
    glUseProgram(programID);

    // Get the model, view, and projection locations
    GLint modelMatLoc = glGetUniformLocation(programID, "modelMat");
    GLuint viewMatLoc = glGetUniformLocation(programID, "viewMat");
    GLuint projMatLoc = glGetUniformLocation(programID, "projMat");

    // Set the key callback function
    glfwSetKeyCallback(window, keyCallback);

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

        // Create view matrix
        glm::mat4 view = glm::lookAt(eye, lookAt, glm::vec3(0.0f, 1.0f, 0.0f));

        // Pass view matrix to shader
        glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Calculate aspect ratio
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspectRatio = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;

        // Create projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), aspectRatio, 0.01f, 50.0f);

        // Pass projection matrix to shader
        glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Draw the scene recursively
        renderScene(meshGLVector, scene->mRootNode, glm::mat4(1.0f), modelMatLoc, 0);

        // Swap buffers and poll for window events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up all MeshGL objects
    for (auto& mgl : meshGLVector) {
        cleanupMesh(mgl);
    }

    cleanupGLFW(window);

    return 0;
}