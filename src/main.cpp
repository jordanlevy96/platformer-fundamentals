/*
ZJ Wood CPE 471 Lab 3 base code
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
#include "Shape.cpp" //linker doesn't work for this ???
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;
shared_ptr<Shape> shape;
double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = glm::vec3(0, 0, -50);
		rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double frametime)
	{
		double ftime = frametime;
		float speed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -1*ftime;
		else if(d==1)
			yangle = 1*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:
	int left = 0;
	int right = 0;
	int up = 0;
	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, pwater;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexColorIDBox, IndexBufferIDBox;

	//texture stuff for water
	GLuint VertexTexBox, Texture;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		/*
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		 */
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) left = 1;
		if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) left = 0;

        if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) right = 1;
        if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) right = 0;

        if (key == GLFW_KEY_UP && action == GLFW_PRESS) up = 1;
        if (key == GLFW_KEY_UP && action == GLFW_RELEASE) up = 0;
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			// back
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0,
			//tube 8 - 11
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			//12 - 15
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0

			
		};
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		//color
		GLfloat cube_colors[] = {
			// front colors
			0, 0.5, 0.3,
            0, 0.5, 0.3,
            0, 0.5, 0.3,
            0, 0.5, 0.3,
			// back colors
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			// tube colors
			0.0, 0.8, 0.8,
            0.0, 0.8, 0.8,
            0.0, 0.8, 0.8,
            0.0, 0.8, 0.8,

            0.0, 0.8, 0.8,
            0.0, 0.8, 0.8,
            0.0, 0.8, 0.8,
            0.0, 0.8, 0.8,
		};
		glGenBuffers(1, &VertexColorIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexColorIDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {
		
			// front
			0, 1, 2,
			2, 3, 0,
			// back
			7, 6, 5,
			5, 4, 7,
			//tube 8-11, 12-15
			8,12,13,
			8,13,9,

			9,13,14,
			9,14,10,

			10,14,15,
			10,15,11,

			11,15,12,
			11,12,8
			
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

		glBindVertexArray(0);

		/** water texture **/

        string resourceDirectory = "../resources" ;
        // Initialize mesh.
        /*shape = make_shared<Shape>();
        //shape->loadMesh(resourceDirectory + "/t800.obj");
        shape->loadMesh(resourceDirectory + "/sphere.obj");
        shape->resize();
        shape->init();*/

        int width, height, channels;
        char filepath[1000];

        //texture 1
        string str = resourceDirectory + "/water.png";
        strcpy(filepath, str.c_str());
        unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("type");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertColor");

        pwater = std::make_shared<Program>();
        pwater->setVerbose(true);
        pwater->setShaderNames(resourceDirectory + "/watervertex.glsl", resourceDirectory + "/waterfrag.glsl");
        if (!pwater->init())
        {
            std::cerr << "pwater shaders failed!" << std::endl;
            exit(1);
        }
        pwater->addUniform("P");
        pwater->addUniform("V");
        pwater->addUniform("M");
        pwater->addUniform("campos");
        pwater->addAttribute("vertPos");
        pwater->addAttribute("vertNor");
        pwater->addAttribute("vertTex");
	}

	void collideObject(float *playerX, float *playerY, float objTop, float objBottom, float objLeft, float objRight) {
	    if (*playerX > objLeft && *playerX < objRight) {
	        //inside x bounds of object
	        if (*playerY < objTop && *playerY > objBottom) {
                //inside y bounds of object
                float distToLeft = *playerX - objLeft;
                float distToRight = objRight - *playerX;
                float distToTop = objTop - *playerY;
                float distToBottom = *playerY - objBottom;

                float smallest = std::min(distToLeft, std::min(distToRight, std::min(distToTop, distToBottom)));

	            //move player to closest edge
                if (smallest == distToLeft) {
                    *playerX = objLeft;
                }
                else if (smallest == distToRight) {
                    *playerX = objRight;
                }
                else if (smallest == distToTop) {
                    *playerY = objTop;
                }
                else if (smallest == distToBottom) {
                    *playerY = objBottom;
                }
	        }
	    }
	}

	void windZone(float *playerX, float *playerY, float windTop, float windBottom,
	              float windLeft, float windRight, string windDir) {
        if (*playerX > windLeft && *playerX < windRight) {
            if (*playerY < windTop && *playerY > windBottom) {
                //inside of wind zone

                if (windDir == "right") {
                    if (left == 1) {
                        *playerX += 0.05; //half speed
                    }
                    else if (right == 1) {
                        *playerX += 0.1; //double speed
                    }
                }
                else if (windDir == "left") {
                    if (left == 1) {
                        *playerX -= 0.1; //double speed
                    }
                    else if (right == 1) {
                        *playerX -= 0.05; //half speed
                    }
                }
            }
        }
	}

	void waterZone(float *playerX, float *playerY, float waterTop, float waterBottom,
	               float waterLeft, float waterRight, bool *canJump, float *jumpTime) {
        if (*playerX > waterLeft && *playerX < waterRight) {
            if (*playerY < waterTop && *playerY > waterBottom) {
                //in water zone
                *canJump = true; //can always jump underwater

                if (left == 1) {
                    *playerX += 0.05; //half speed
                }
                else if (right == 1) {
                    *playerX -= 0.05; //half speed
                }

                if (up == 1) {
                    *jumpTime = 20; //jump longer in (and out of) water
                }

                if (*jumpTime >= 0) {
                    //is jumping
                    *playerY += 0.05; //fall slower in water
                }
            }
        }
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		double frametime = get_last_elapsed_time();
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = glm::mat4(1);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0;
		w += 0.01;//rotation angle
		static float t = 0;
		t += 0.01;

		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

		int type = 1; //ground
		
		// Draw the box using GLSL.
		prog->bind();

		V = mycam.process(frametime);
		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform1i(prog->getUniform("type"), type);

		glBindVertexArray(VertexArrayID);
		//actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);



        /*************** ground **************/
        mat4 groundScale = glm::scale(mat4(1), vec3(30, 1, 5));
        mat4 groundT = glm::translate(mat4(1), vec3(0, -10, 0));
		M = groundT * groundScale;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

        /*************** obstacle ************/
        mat4 obTrans = glm::translate(mat4(1), vec3(15, -5, 0));
        mat4 obScale = glm::scale(mat4(1), vec3(4, 4, 5));
        float obstacleLeft = 10.0f, obstacleRight = 20.0f, obstacleTop = 0.1, obstacleBottom = -9.0f;

        M = obTrans * obScale;
        type = 2;
        glUniform1i(prog->getUniform("type"), type);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

        /*************** obstacle2 ************/
        mat4 obTrans2 = glm::translate(mat4(1), vec3(100, -5, 0));
        mat4 obScale2 = glm::scale(mat4(1), vec3(2, 2, 5));
        float obstacle2Left = 97.0f, obstacle2Right = 103.0f, obstacle2Top = -2, obstacle2Bottom = -8;

        M = obTrans2 * obScale2;
        type = 2;
        glUniform1i(prog->getUniform("type"), type);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

        /*************** obstacle3 ************/
        mat4 obTrans3 = glm::translate(mat4(1), vec3(80, 3, 0));
        mat4 obScale3 = glm::scale(mat4(1), vec3(8, 2, 5));
        float obstacle3Left = 71.0f, obstacle3Right = 89.0f, obstacle3Top = 6, obstacle3Bottom = 0;

        M = obTrans3 * obScale3;
        type = 2;
        glUniform1i(prog->getUniform("type"), type);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

        /*************** obstacle4 ************/
        mat4 obTrans4 = glm::translate(mat4(1), vec3(47, 5, 0));
        mat4 obScale4 = glm::scale(mat4(1), vec3(2, 2, 5));
        float obstacle4Left = 44.0f, obstacle4Right = 50.0f, obstacle4Top = 8, obstacle4Bottom = 2;

        M = obTrans4 * obScale4;
        type = 4;
        glUniform1i(prog->getUniform("type"), type);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

        /************* wind zone *************/

        mat4 windScale = glm::scale(mat4(1), vec3(5, 0.1, 0));
        mat4 windRot = glm::rotate(mat4(1), 0.3f, vec3(0, 0, 1));
        mat4 windTrans;

        type = 3;
        glUniform1i(prog->getUniform("type"), type);

        for (int i = 0; i < 5; i++) {
            windTrans = glm::translate(mat4(1), vec3(60, i * 5 - 15, 0));
            M = windRot * windTrans * windScale;
            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

            windTrans = glm::translate(mat4(1), vec3(63, i * 5 - 17, 0));
            M = windRot * windTrans * windScale;
            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
        }

        /************ water *****************/

        prog->unbind();

        pwater->bind();

        //send the matrices to the shaders

        mat4 waterScale = glm::scale(mat4(1), vec3(50, 10, 2));
        mat4 waterTrans = glm::translate(mat4(1), vec3(80, -18.5, -1));
        float waterTop = -8.5, waterBottom = -80, waterLeft = 30, waterRight = 180;

        M = waterTrans * waterScale;

        glUniformMatrix4fv(pwater->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(pwater->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(pwater->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(pwater->getUniform("campos"), 1, &mycam.pos[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glDisable(GL_DEPTH_TEST);
//        shape->draw(pwater, false);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);


        glEnable(GL_DEPTH_TEST);

        pwater->unbind();

        prog->bind();

        /*************** player **************/
        static float playerX = 0;
        static float playerY = 0;
        static bool canJump = true;
        static float jumpTime = 0;
        static float lastY = 0;

        lastY = playerY;

        if (left == 1) {
            playerX -= 0.1;
        }
        if (right == 1) {
            playerX += 0.1;
        }

        if (up == 1 && canJump) {
            canJump = false;
            jumpTime = 10;
        }

        if (jumpTime >= 0) {
           playerY += 0.1;
           jumpTime -= 0.1;
        }
        else { //not jumping; gravity brings you down
            playerY -= 0.1;
        }

        collideObject(&playerX, &playerY, -8, -12, -100, waterLeft); //floor
        collideObject(&playerX, &playerY, obstacleTop, obstacleBottom, obstacleLeft, obstacleRight);
        collideObject(&playerX, &playerY, obstacle2Top, obstacle2Bottom, obstacle2Left, obstacle2Right);
        collideObject(&playerX, &playerY, obstacle3Top, obstacle3Bottom, obstacle3Left, obstacle3Right);
        collideObject(&playerX, &playerY, obstacle4Top, obstacle4Bottom, obstacle4Left, obstacle4Right);

        windZone(&playerX, &playerY, 100, -0, 60, 70, "left");

        waterZone(&playerX, &playerY, waterTop, waterBottom, waterLeft, waterRight, &canJump, &jumpTime);

        if (lastY == playerY) {
            canJump = true;
        }

        mycam.pos.x = -playerX;
        mycam.pos.y = -playerY;

        glm::mat4 PlayerLocation = glm::translate(mat4(1), vec3(playerX, playerY, 0));

        M = PlayerLocation * S;
        type = 0;
        glUniform1i(prog->getUniform("type"), type);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);


		
		glBindVertexArray(0);

		prog->unbind();

	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
