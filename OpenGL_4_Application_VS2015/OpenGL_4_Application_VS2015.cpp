//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include <math.h>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"
#include "SkyBox.hpp"

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

float lastX = 320, lastY = 320;  //mouse
float yaw, pitch;

gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 2.0f, -10.0f));
float cameraSpeed = 0.05f;

bool pressedKeys[1024];
float angle = 0.0f;

gps::Model3D ground;
gps::Model3D bridge;
gps::Model3D stoneWall;
gps::Model3D house;
gps::Model3D flowers;
gps::Model3D well;
gps::Model3D trees[8];
gps::Model3D rock;
gps::Model3D tower;
gps::Model3D wagon;
gps::Model3D road;
gps::Model3D dog;
gps::Model3D water;
gps::Model3D dragon;
gps::Model3D sun;

gps::Shader myCustomShader;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

gps::Shader sunShader;
gps::Shader rainShader;

std::vector<const GLchar*> faces;

GLfloat lightAngle;

gps::BoundingBox boundingBoxes[100];

GLuint verticesVBO;
GLuint verticesEBO;
GLuint objectVAO;
GLint texture;

bool isRaining = false;

GLfloat translateOnY = 0.0f;
GLfloat advance = 0.0f;
GLfloat dragonAngle = 0.0f;
GLfloat speed = 0.5f;

glm::mat4 modelDragon = glm::mat4(1.0f);

struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	GLfloat life;
	GLfloat fadeSpeed;

	Particle()
		: position((float)((rand() % 100) - 60), 25.0, (float)((rand() % 100) - 60)), velocity(glm::vec3(0.5f, 1.5f, 0.5f)), life(10.0f), fadeSpeed(float(rand() % 100) / 1000.0f + 0.005f) { }
};

GLuint NR_OF_PARTICLES = 2000;
Particle particles[2000];

void respawnParticle(Particle& particle)
{
	particle.position.x = (float)(rand() % 100) - 60;
	particle.position.y = 25.0;
	particle.position.z = (float)(rand() % 100) - 60;
	particle.life = 10.0f;
	particle.velocity = glm::vec3(1.5f);
	particle.fadeSpeed = float(rand() % 100) / 1000.0f + 0.005f;
}

void UpdateParticleSystem(GLfloat timePassed, GLuint newParticles)
{
	for (GLuint i = 0; i < NR_OF_PARTICLES; i++)
	{
		Particle& p = particles[i];
		p.life -= p.fadeSpeed;

		if (p.life > 0.0f)
		{
			p.position -= p.velocity * timePassed;

			//wind?
			/*p.position.x -= timePassed;
			p.position.y -= timePassed;
			p.position.z += timePassed;*/
		}
	}
}

void DrawRain()
{
	rainShader.useShaderProgram();

	GLint viewLoc = glGetUniformLocation(rainShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	GLint projLoc = glGetUniformLocation(rainShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(rainShader.shaderProgram, "rainDropTexture"), 0);
	glBindTexture(GL_TEXTURE_2D, texture);

	GLuint offsetLoc = glGetUniformLocation(rainShader.shaderProgram, "offset");

	for (int i = 0; i < NR_OF_PARTICLES; i++) {
		Particle particle = particles[i];

		if (particle.life > 0.0f)
		{
			glUniform3fv(offsetLoc, 1, glm::value_ptr(particle.position));
			glBindVertexArray(objectVAO);
			glDrawArrays(GL_LINES, 0, NR_OF_PARTICLES);
			glBindVertexArray(0);
		}
		else respawnParticle(particles[i]);
	}

	UpdateParticleSystem(0.1f, 5);
}

void initParticles()
{
	for (GLuint i = 0; i < NR_OF_PARTICLES; ++i)
		particles[i] = Particle();

	GLfloat rainDropFormat[] = {
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

		-0.25f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.25f, 0.5f, 0.0f, 1.0f, 1.0f,
		-0.25f, 0.5f, 0.0f, 1.0f, 0.0f
	};
	
	glGenVertexArrays(1, &objectVAO);
	glBindVertexArray(objectVAO);

	glGenBuffers(1, &verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rainDropFormat), rainDropFormat, GL_STATIC_DRAW);

	//vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0); //astea trebuie schimbate
	glEnableVertexAttribArray(0);

	//vertex texture
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	sunShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(sunShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 0.1f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 0.1f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_P]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_O]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_R]) {
		isRaining = !isRaining;
	}

	if (glfwGetKey(glWindow, GLFW_KEY_UP)) {
		advance -= speed;
	}

	if (glfwGetKey(glWindow, GLFW_KEY_DOWN)) {
		translateOnY -= speed;
	}

	if (glfwGetKey(glWindow, GLFW_KEY_LEFT)) {
		dragonAngle += 0.02;
	}

	if (glfwGetKey(glWindow, GLFW_KEY_RIGHT)) {
		dragonAngle -= 0.02;
	}

	if (glfwGetKey(glWindow, GLFW_KEY_U)) {
		translateOnY += speed;
	}

	//if (pressedKeys[GLFW_KEY_J]) {

	//	lightAngle += 0.3f;
	//	if (lightAngle > 360.0f)
	//		lightAngle -= 360.0f;
	//	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	//	myCustomShader.useShaderProgram();
	//	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	//}

	//if (pressedKeys[GLFW_KEY_L]) {
	//	lightAngle -= 0.3f;
	//	if (lightAngle < 0.0f)
	//		lightAngle += 360.0f;
	//	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	//	myCustomShader.useShaderProgram();
	//	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	//}
}

//glm::mat4 computeLightSpaceTrMatrix()
//{
//	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
//	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
//
//	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
//	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));
//
//	return lightProjection * lightView;
//}

gps::BoundingBox createBoundingBox(gps::Model3D object) {
	gps::BoundingBox boundingBox;

	for (int i = 0; i < object.meshes.size(); i++) {
		for (gps::Vertex vertex : object.meshes[i].vertices) {
			if (vertex.Position.x < boundingBox.min.x) {
				boundingBox.min.x = vertex.Position.x;
			}
			else if (vertex.Position.x > boundingBox.max.x) {
				boundingBox.max.x = vertex.Position.x;
			}

			if (vertex.Position.y < boundingBox.min.y) {
				boundingBox.min.y = vertex.Position.y;
			}
			else if (vertex.Position.y > boundingBox.max.y) {
				boundingBox.max.y = vertex.Position.y;
			}

			if (vertex.Position.z < boundingBox.min.z) {
				boundingBox.min.z = vertex.Position.z;
			}
			else if (vertex.Position.z > boundingBox.max.z) {
				boundingBox.max.z = vertex.Position.z;
			}
		}
	}
	
	return boundingBox;
}

void moveDragon() {
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	modelDragon = glm::translate(modelDragon, glm::vec3(0, translateOnY, advance));
	modelDragon = glm::rotate(modelDragon, dragonAngle, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDragon));

	myCustomShader.useShaderProgram();
	dragon.Draw(myCustomShader);

	dragonAngle = 0;
	translateOnY = 0;
	advance = 0;
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	faces.push_back("textures/skybox/lakes_rt.tga");
	faces.push_back("textures/skybox/lakes_lf.tga");
	faces.push_back("textures/skybox/lakes_up.tga");
	faces.push_back("textures/skybox/lakes_dn.tga");
	faces.push_back("textures/skybox/lakes_bk.tga");
	faces.push_back("textures/skybox/lakes_ft.tga");

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels()
{
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	bridge = gps::Model3D("objects/bridge/bridge.obj", "objects/bridge/");
	stoneWall = gps::Model3D("objects/stone_wall/stone_wall.obj", "objects/stone_wall/");
	house = gps::Model3D("objects/house/house.obj", "objects/house/");
	flowers = gps::Model3D("objects/flowers/flowers.obj", "objects/flowers/");
	well = gps::Model3D("objects/well/well.obj", "objects/well/");
	trees[1] = gps::Model3D("objects/trees/tree1.obj", "objects/trees/");
	trees[2] = gps::Model3D("objects/trees/tree2.obj", "objects/trees/");
	trees[3] = gps::Model3D("objects/trees/tree3.obj", "objects/trees/");
	trees[4] = gps::Model3D("objects/trees/tree4.obj", "objects/trees/");
	trees[5] = gps::Model3D("objects/trees/tree5.obj", "objects/trees/");
	trees[6] = gps::Model3D("objects/trees/tree6.obj", "objects/trees/");
	trees[7] = gps::Model3D("objects/trees/tree7.obj", "objects/trees/");
	rock = gps::Model3D("objects/rock/rock.obj", "objects/rock/");
	tower = gps::Model3D("objects/tower/tower.obj", "objects/tower/");
	wagon = gps::Model3D("objects/wagon/wagon.obj", "objects/wagon/");
	road = gps::Model3D("objects/road/road.obj", "objects/road/");
	dog = gps::Model3D("objects/dog/dog.obj", "objects/dog/");
	water = gps::Model3D("objects/water/water.obj", "objects/water/");
	dragon = gps::Model3D("objects/dragon/dragon.obj", "objects/dragon/");
	sun = gps::Model3D("objects/sun/sun.obj", "objects/sun/");
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");

	sunShader.loadShader("shaders/sunShader.vert", "shaders/sunShader.frag");
	
	rainShader.loadShader("shaders/rainShader.vert", "shaders/rainShader.frag");
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	sunShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(sunShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
	glm::value_ptr(projection));
}

void initBoundingBoxes() {
	myCamera.boundingBoxes[0] = createBoundingBox(bridge);
	myCamera.boundingBoxes[1] = createBoundingBox(stoneWall);
	myCamera.boundingBoxes[2] = createBoundingBox(house);
	myCamera.boundingBoxes[3] = createBoundingBox(well);
	myCamera.boundingBoxes[4] = createBoundingBox(rock);
	myCamera.boundingBoxes[5] = createBoundingBox(tower);
	myCamera.boundingBoxes[6] = createBoundingBox(wagon);
	myCamera.boundingBoxes[7] = createBoundingBox(dog);
	myCamera.boundingBoxes[8] = createBoundingBox(dragon);
	myCamera.boundingBoxes[9] = createBoundingBox(road);
	myCamera.boundingBoxes[10] = createBoundingBox(ground);

	/*int objectCount = 11;
	for (int tree = 1; tree < 8; tree++, objectCount++) {
		myCamera.boundingBoxes[objectCount] = createBoundingBox(trees[tree]);
	}*/
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	myCustomShader.useShaderProgram();
	processMovement();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	ground.Draw(myCustomShader);
	bridge.Draw(myCustomShader);
	stoneWall.Draw(myCustomShader);
	house.Draw(myCustomShader);
	flowers.Draw(myCustomShader);
	well.Draw(myCustomShader);
	trees[1].Draw(myCustomShader);
	trees[2].Draw(myCustomShader);
	trees[3].Draw(myCustomShader);
	trees[4].Draw(myCustomShader);
	trees[5].Draw(myCustomShader);
	trees[6].Draw(myCustomShader);
	trees[7].Draw(myCustomShader);
	rock.Draw(myCustomShader);
	tower.Draw(myCustomShader);
	wagon.Draw(myCustomShader);
	road.Draw(myCustomShader);
	dog.Draw(myCustomShader);
	water.Draw(myCustomShader);

	moveDragon();

	skyboxShader.useShaderProgram();
	mySkyBox.Draw(skyboxShader, view, projection);

	//draw a white cube around the light
	sunShader.useShaderProgram();
	/*glUniformMatrix4fv(glGetUniformLocation(sunShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, lightDir);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(glGetUniformLocation(sunShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));*/

	sun.Draw(sunShader);
	
	if (isRaining) {
		DrawRain();
	}
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();	
	initBoundingBoxes();

	initParticles();

	texture = gps::Model3D::ReadTextureFromFile("textures\\particle.jpg");

	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
