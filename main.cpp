// Flight Game
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>
#include <unistd.h>
#include <shader.h>
#include <Model.h>
#include <text.h>

// Function prototypes
GLFWwindow *glAllInit();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void render();
void handle_keys();
void levelUp();

// Global variable
GLFWwindow *mainWindow = NULL;
Shader *globalShader = NULL;
Shader *modelShader = NULL;
Shader *textShader = NULL;

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// Array of key states
bool keyStates[256];

// Map array
int level = 1;
#define MapHeight 20
#define MapWidth 5
int _map[MapHeight][MapWidth];

// world
glm::vec3 planePos = glm::vec3(0.0f, 1.0f, 0.0f);

// camera
glm::vec3 cameraDiflection = glm::vec3(0.0f, 1.0f, -3.0f);
glm::vec3 cameraPos    = planePos + cameraDiflection;
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraUp 		= glm::vec3(0.0f, 1.0f,  0.0f);

// for lighting
glm::vec3 lightSize(0.2f, 0.2f, 0.2f);
glm::vec3 lightPos(0.0f, 10.0f, -5.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
float ambientStrenth = 30.0f;
float specularStrength = 1.0f;
float specularPower = 64.0f;

// Objects
Model *mPlane;
Model *mFloor;
Model *mRing;
Model *mSkybox;

// Text font
Text *text = NULL;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float cameraSpeed = 2.0f;

float yaw   = 0.0f;    // yaw is initialized to -90.0 degrees since a yaw of 0.0 results
                         // in a direction vector pointing to the right
                         // so we initially rotate a bit to the left.
float pitch =  0.0f;
float fov		= 70.0f;

// !
int s = 0;
int ringCount = 0;
int state = 0;
bool endgame = false;
int totalscore = 0;
bool play = false;
bool finishline = false;
bool showinfo = 0;
bool meshview = 0;

float planeroty = 0.0f;
float temproty = 0.0f;
int b_planeroty = 0;
float planerotx = 0.0f;
int b_planerotx = 0;
float ringrot = 0.0f;

int main() {
	mainWindow = glAllInit();

	//
	// main program
	//
	// Init Key states to false
	for (int i = 0; i < 256; i++)
		keyStates[i] = false;
	// Init map to 0
	for (int i = 0; i < MapHeight; i++)
		for (int j = 0; j < MapWidth; j++)
			_map[i][j] = 0;
	// Loading level
	if(level == 1) {
		// Init the map array from __level__.txt
		using namespace std;
		ifstream file("res/courses/tutorial.txt");

		for (int i = 0; i < MapHeight; i++)
			for (int j = 0; j < MapWidth; j++)
				file >> _map[i][j];
	}

	// shader loading and compile (by calling the constructor)
	globalShader = new Shader("res/shaders/global.vs", "res/shaders/global.fs");
	modelShader = new Shader("res/shaders/modelLoading.vs", "res/shaders/modelLoading.fs");
	textShader = new Shader("res/shaders/text.vs", "res/shaders/text.fs");

	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float) SCR_HEIGHT, 0.001f, 1000.0f);
	modelShader->use();
	modelShader->setMat4("projection", projection);
	modelShader->setVec3("objectColor", objectColor);
	modelShader->setVec3("lightColor", lightColor);
	modelShader->setVec3("lightPos", lightPos);
	modelShader->setVec3("viewPos", cameraPos);
	modelShader->setFloat("ambientStrenth", ambientStrenth);
	modelShader->setFloat("specularStrength", specularStrength);
	modelShader->setFloat("specularPower", specularPower);

	// LOAD MODEL
	mFloor = new Model((GLchar *)"res/models/floor/floor.obj");
	mPlane = new Model((GLchar *)"res/models/plane/plane.obj");
	mRing = new Model((GLchar *)"res/models/ring/ring.obj");
	mSkybox = new Model((GLchar *)"res/models/skybox/skybox.obj");

	// text
	text = new Text((char*)"res/fonts/arial.ttf", textShader, SCR_WIDTH, SCR_HEIGHT);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(mainWindow)) {
		render();
		glfwPollEvents();
	}
	
	// glfw: terminate, clearing all previous allocated GLFW resources.
	// ---------------------------------------------------------------
	glfwTerminate();
	return 0;
}

GLFWwindow* glAllInit() {
	GLFWwindow *window;

	// glfw: initialize and configure
	if (!glfwInit()) {
		printf("GLFW initialization failed!\n");
		glfwTerminate();
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Camera FPS", NULL, NULL);
	if (window == NULL) {
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	// OpenGL states
	glClearColor(0.529f, 0.808f, 0.929f, 0.8f);
	glEnable(GL_DEPTH_TEST);

	// Allow modern extentsion features
//	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		printf("GLEW initialization failed!\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(-1);
	}

	// Set OpenGL options
//	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
}

// Render
void render() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	handle_keys();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glm::ivec3 temp;

	temp.x = round(planePos.x);
	temp.z = round(planePos.z);

	if(_map[temp.z][temp.x] > 0 &&
		( planePos.y > (_map[temp.z][temp.x] / float(100) * 2 - 0.4f)) &&
		( planePos.y < (_map[temp.z][temp.x] / float(100) * 2 + 0.4f)) ){
		_map[temp.z][temp.x] = 0;
		s++;
		totalscore++;
//		printf("%d %d %f %f %f\n", temp.x, temp.z, planePos.x, planePos.y, planePos.z);
	}

	ringCount = 0;

	for (int i = 0; i < MapHeight; i++)
		for (int j = 0; j < MapWidth; j++)
			if(_map[i][j] > 0) ringCount++;

	if( planePos.z > 20 && planePos.z < 21) {
		finishline = true;
		play = false;
	}

//	printf("%f %f %f\n", planePos.x, planePos.y, planePos.z);
	if ( planePos.x < -2 || planePos.y < 0.0f || planePos.z < -2 ||
			planePos.x > MapWidth + 1 || planePos.y > 2 || planePos.z > MapHeight + 1) {
		endgame = true;
	}

	// Ring Rotate
	ringrot++;

	if(b_planeroty == 1)
		if(temproty < 90)
			temproty++;

	if(b_planeroty == -1)
		if(temproty > -90)
			temproty--;

	if(b_planeroty != -1 && b_planeroty != 1 && temproty != 0)
		temproty += temproty > 0 ? -1 : 1; // Animate rotation back to normal

	if(b_planerotx == 1)
		if(planerotx < 60)
			planerotx++;

	if(b_planerotx == -1)
		if(planerotx > -60)
			planerotx--;

	if(b_planerotx != -1 && b_planerotx != 1 && planerotx != 0)
		planerotx += planerotx > 0 ? -1 : 1; // Animate rotation back to normal
		
		
	// pass projection matrix to shader (note that in this case it could change every frame)
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float) SCR_HEIGHT, 0.001f, 1000.0f);
	
	// camera/view tranformation
	glm::mat4 view = glm::lookAt(cameraPos, planePos, cameraUp);

	// Model drawing
	glm::mat4 model;

	globalShader->use();

	globalShader->setMat4("projection", projection);
	globalShader->setMat4("view", view);

	// -------------------- Skybox
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(800.0f, 800.0f, 800.0f));
	globalShader->setMat4("model", model);
	mSkybox->Draw(globalShader);

	modelShader->use();

	modelShader->setMat4("projection", projection);
	modelShader->setMat4("view", view);

	// Update lightPos
	modelShader->setVec3("lightPos", lightPos);

	// -------------------- Floor
	for (int i = -1; i < MapHeight+1; i++) {
		for (int j = -1; j < MapWidth+1; j++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3((float)(j), -0.3f, (float)(i)));
			model = glm::scale(model, glm::vec3(1.0f, 0.08f, 1.0f));
			modelShader->setMat4("model", model);
			mFloor->Draw(modelShader);

			if (i >= 0 && i < MapHeight && j >= 0 && j < MapWidth && _map[i][j] > 0) {
				// -------------------- Ring
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3((float)(j), (float)((_map[i][j] / (float)100) * 2), (float)(i)));
				model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
				model = glm::rotate(model, glm::radians(90.0f),  glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(ringrot),  glm::vec3(0.0f, 0.0f, 1.0f));
				modelShader->setMat4("model", model);
				mRing->Draw(modelShader);
			}
		}
	}
	
	// -------------------- Plane
	model = glm::mat4(1.0f);
	model = glm::translate(model, planePos);
	model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
	model = glm::rotate(model, glm::radians(-planerotx),  glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-temproty),  glm::vec3(0.0f, 0.0f, 1.0f));
	modelShader->setMat4("model", model);
	mPlane->Draw(modelShader);
	
	// Text
	char score[128];
	char ringsLeft[128];
	char levelNow[128];
	char tempText[128];

	textShader->use();

	if (showinfo == 1) {
		snprintf(tempText, 128, "Manually Level Select(1-6)");
		text->RenderText(tempText, 5.0f, 50.0f, 0.26f, glm::vec3(1.0f, 1.0f, 1.0f));
		snprintf(tempText, 128, "PLAY/PAUSE([SPACE])");
		text->RenderText(tempText, 5.0f, 35.0f, 0.26f, glm::vec3(1.0f, 1.0f, 1.0f));
		snprintf(tempText, 128, "Plane Move(WSAD)");
		text->RenderText(tempText, 5.0f, 20.0f, 0.26f, glm::vec3(1.0f, 1.0f, 1.0f));
		snprintf(tempText, 128, "Mesh View On(Q) Reset(^Q)");
		text->RenderText(tempText, 5.0f, 5.0f, 0.26f, glm::vec3(1.0f, 1.0f, 1.0f));
	}

	snprintf(levelNow, 128, "Level %i", level);
	text->RenderText(levelNow, 5.0f, SCR_HEIGHT - 15.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f));
	snprintf(score, 128, "Score: %d", totalscore);
	text->RenderText(score, 5.0f, SCR_HEIGHT - 32.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f));
	snprintf(ringsLeft, 128, "Rings Left: %i/%i", ringCount, ringCount + s);
	text->RenderText(ringsLeft, 5.0f, SCR_HEIGHT - 48.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f));
	snprintf(tempText, 128, "More Info (I)");
	text->RenderText(tempText, 5.0f, SCR_HEIGHT - 64.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f));

	if (!play){
		snprintf(tempText, 128, "ll");
		text->RenderText(tempText, SCR_WIDTH - 40.0f, SCR_HEIGHT - 40.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	}
	if (state == 1) {
		snprintf(tempText, 128, "Level Completed");
		text->RenderText(tempText, 5.0f, SCR_HEIGHT - 96.0f, 0.6f, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (state == 2) {
		snprintf(tempText, 128, "Game Over");
		text->RenderText(tempText, 5.0f, SCR_HEIGHT - 96.0f, 0.6f,glm::vec3(1.0f, 0.0f, 0.0f));
	}

	// reset
	b_planeroty = 0;
	b_planerotx = 0;
	
	glfwSwapBuffers(mainWindow);

	// UPDATE level

	if (state == 1) {
		sleep(2);
		level++;
		levelUp();
	}

	if (state == 2) {
		sleep(2);
		level = level;
		levelUp();
	}

	state = 0;

	if (finishline)
		if(ringCount == 0)
			state = 1;
		else
			state = 2;

	if (endgame)
		state = 2;
}

// glfw: whenever the window size changes (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	// Make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger thatn specified on retina displays.
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scnacode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	else if (key == GLFW_KEY_I && action == GLFW_PRESS)
		showinfo = !showinfo;
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		play = !play;
	else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		meshview = !meshview;
		if (meshview)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}	else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		level = 1;
		levelUp();
	}	else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		level = 2;
		levelUp();
	}	else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		level = 3;
		levelUp();
	}	else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		level = 4;
		levelUp();
	}	else if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		level = 5;
		levelUp();
	}	else if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
		level = 6;
		levelUp();
	}	else if (action == GLFW_PRESS)
		keyStates[key] = 1;
	else if (action == GLFW_RELEASE)
		keyStates[key] = 0;
}

void handle_keys() {
	if(play) {
		float cameraDelta = cameraSpeed * deltaTime;

		glm::vec3 prevCameraPos = cameraPos;

		cameraPos += cameraDelta * cameraFront;

		if (keyStates[GLFW_KEY_W]) {
			b_planerotx = 1;
			cameraPos += cameraUp * cameraDelta;
		}
		if (keyStates[GLFW_KEY_S]) {
			b_planerotx = -1;
			cameraPos -= cameraUp * cameraDelta;
		}

		if (keyStates[GLFW_KEY_A]) {
			b_planeroty = 1;
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraDelta;
		}

		if (keyStates[GLFW_KEY_D]) {
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraDelta;
			b_planeroty = -1;
		}

		if(prevCameraPos != cameraPos) {
			planePos = cameraPos - cameraDiflection;
			lightPos.z = planePos.z - 5;
		}
	}
}

// Next level
void levelUp() {
	// Reset every thing
	planePos = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraPos = planePos + cameraDiflection;

	s = 0;
	state = 0;
	ringCount = 0;
	totalscore = 0;
	endgame = false;
	play = false;
	finishline = false;

	showinfo = 0;
	meshview = 0;

	planeroty = 0.0f;
	temproty = 0.0f;
	b_planeroty = 0;
	planerotx = 0.0f;
	b_planerotx = 0;
	ringrot = 0.0f;

	// Loading level tutorial
	if(level == 1) {
		using namespace std;
		ifstream file("res/courses/tutorial.txt");

		for (int i = 0; i < MapHeight; i++)
			for (int j = 0; j < MapWidth; j++)
				file >> _map[i][j];
	}

	// Loading level 1
	if(level == 2) {
		using namespace std;
		ifstream file("res/courses/course1.txt");

		for (int i = 0; i < MapHeight; i++)
			for (int j = 0; j < MapWidth; j++)
				file >> _map[i][j];
	}

	// Loading level 2
	if(level == 3) {
		using namespace std;
		ifstream file("res/courses/course2.txt");

		for (int i = 0; i < MapHeight; i++)
			for (int j = 0; j < MapWidth; j++)
				file >> _map[i][j];
	}

	// Loading level 3
	if(level == 4) {
		using namespace std;
		ifstream file("res/courses/course3.txt");

		for (int i = 0; i < MapHeight; i++)
			for (int j = 0; j < MapWidth; j++)
				file >> _map[i][j];
	}

	// Loading level 4
	if(level == 5) {
		using namespace std;
		ifstream file("res/courses/course4.txt");

		for (int i = 0; i < MapHeight; i++)
			for (int j = 0; j < MapWidth; j++)
				file >> _map[i][j];
	}

	// Loading Ring Rush -- Just for fun
	if(level == 6)
		for (int i = 0; i < MapHeight; i++)
			for (int j = 0; j < MapWidth; j++)
				_map[i][j] = 50;
}
