#include "stdafx.h"
#include <fstream>
#include <GL/glew.h>
#include <freeglut.h>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <list>
using namespace std;

#include "shaders.h"

#include "point3d.h"
#include "vector3d.h"
#include "myObject3D.h"

// width and height of the window.
int Glut_w = 600, Glut_h = 400; 

//Variables and their values for the camera setup.
myPoint3D camera_eye(0,2,4);
myVector3D camera_up(0, 1, 0);
myVector3D camera_forward(0, 0, -0.5);


float fovy = 90;
float zNear = 0.2;
float zFar = 6000;

int button_pressed = 0; // 1 if a button is currently being pressed.
int GLUTmouse[2] = { 0, 0 };

GLuint vertexshader, fragmentshader, shaderprogram1; // shaders

GLuint renderStyle = 0;
GLuint mylightType =0;
GLuint renderStyle_loc;
GLuint projection_matrix_loc;
GLuint view_matrix_loc;
GLuint normal_matrix_loc;
GLuint mylightColor_loc;
GLuint mylightPosition_loc;
GLuint mylightDirection_loc;
GLuint mylightType_loc;
GLuint buffers[6];

vector<GLfloat> vertices;
vector<GLfloat> normals;
vector<GLuint> indices;

myObject3D me, *apple;
std::vector<myObject3D> objects;

//these variables control the roration and movement of the camera. When non-zero camera is moving, when zero camera is still
int deltaAngle = 0;
int deltaMove = 0;

bool collision(myObject3D obj, myPoint3D deplacement);

//This function is called when a mouse button is pressed.
void mouse(int button, int state, int x, int y)
{
  // Remember button state 
  button_pressed = (state == GLUT_DOWN) ? 1 : 0;

   // Remember mouse position 
  GLUTmouse[0] = x;
  GLUTmouse[1] = Glut_h - y;
}

//This function is called when the mouse is dragged.
void mousedrag(int x, int y)
{
  // Invert y coordinate
  y = Glut_h - y;

  //change in the mouse position since last time
  int dx = x - GLUTmouse[0];
  int dy = y - GLUTmouse[1];

  GLUTmouse[0] = x;
  GLUTmouse[1] = y;

  if (dx == 0 && dy == 0) return;
  if (button_pressed == 0) return;

  double vx = (double) dx / (double) Glut_w;
  double vy = (double) dy / (double) Glut_h;
  double theta = 4.0 * (fabs(vx) + fabs(vy));

  myVector3D camera_right = camera_forward.crossproduct(camera_up);
  camera_right.normalize();

  myVector3D tomovein_direction = -camera_right*vx + -camera_up*vy;

  myVector3D rotation_axis = tomovein_direction.crossproduct(camera_forward);
  rotation_axis.normalize();
  
  camera_forward.rotate(rotation_axis, theta);
  
  camera_up.rotate(rotation_axis, theta);
  camera_eye.rotate(rotation_axis, theta);
 
  camera_up.normalize();
  camera_forward.normalize();

  glutPostRedisplay();
}

void mousedrag2(int x, int y)
{
	// Invert y coordinate
	y = Glut_h - y;

	//change in the mouse position since last time
	int dx = x - GLUTmouse[0];
	int dy = y - GLUTmouse[1];

	GLUTmouse[0] = x;
	GLUTmouse[1] = y;

	if (dx == 0 && dy == 0) return;
	if (button_pressed == 0) return;

	double vx = (double)dx / (double)Glut_w;
	double vy = (double)dy / (double)Glut_h;
	double theta = 4.0 * (fabs(vx) + fabs(vy));

	myVector3D camera_right = camera_forward.crossproduct(camera_up);
	camera_right.normalize();

	myVector3D tomovein_direction = -camera_right*vx + -camera_up*vy;

	myVector3D rotation_axis = tomovein_direction.crossproduct(camera_forward);
	rotation_axis.normalize();

	camera_forward.rotate(rotation_axis, theta);

	camera_up.normalize();
	camera_forward.normalize();

	glutPostRedisplay();
}

void mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
		camera_eye += camera_forward * 0.02;
	else
		camera_eye += -camera_forward * 0.02;
	glutPostRedisplay();
}

//This function is called when a key is pressed.
void keyboard(unsigned char key, int x, int y) {
	switch(key) {
	case 27:  // Escape to quit
		exit(0) ;
        break ;
	case 'w':
		renderStyle = (renderStyle+1)%3;
		glUniform1i(renderStyle_loc, renderStyle) ; 
		break;
	case 'r':
		camera_eye = myPoint3D(0,2,4);
		camera_up = myVector3D(0,1,0);
		camera_forward = myVector3D(0,0,-0.5);
		break;
	case 'p':
		mylightType = (mylightType+1)%3;
		break;
	}
	glutPostRedisplay();
}

//This function is called when an arrow key is pressed.
void keyboard2(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_UP:
		camera_eye += camera_forward;
		break;
	case GLUT_KEY_DOWN:
		camera_eye += -camera_forward;
		break;
	case GLUT_KEY_LEFT:
		camera_up.normalize();
		camera_forward.rotate(camera_up, 0.1);
		camera_forward.normalize();
		break;
	case GLUT_KEY_RIGHT:
		camera_up.normalize();
		camera_forward.rotate(camera_up, -0.1);
		camera_forward.normalize();
		break;
	}
	glutPostRedisplay();
}

void pressKey(unsigned char key, int x, int y) {
	switch (key) {
		case 'z': deltaMove = 1; break;
		case 's': deltaMove = -1; break;
		case 'q': deltaAngle = 1; break;
		case 'd': deltaAngle = -1; break;
		case 27:  // Escape to quit
			exit(0);
			break;
		case 'w':
			renderStyle = (renderStyle + 1) % 3;
			glUniform1i(renderStyle_loc, renderStyle);
			break;
		case 'r':
			camera_eye = myPoint3D(0, 2, 4);
			camera_up = myVector3D(0, 1, 0);
			camera_forward = myVector3D(0, 0, -0.5);
			break;
		case 'p':
			mylightType = (mylightType + 1) % 3;
			break;
		case 'e':
			if (collision(*apple, myPoint3D(0, 0, 0) + camera_forward * 0.5) && apple != NULL)
			{
				objects.erase(objects.begin());
				apple = NULL;
			}
	}
	glutPostRedisplay();
}

void releaseKey(unsigned char  key, int x, int y) {
	switch (key) {
		case 'z': deltaMove = 0; break;
		case 's': deltaMove = 0; break;
		case 'q': deltaAngle = 0; break;
		case 'd': deltaAngle = 0; break;
	}
	glutPostRedisplay();
}

myPoint3D deplacement() {
	return myPoint3D(0, 0, 0) + camera_forward * 0.012 * deltaMove;
}

bool collision(myObject3D obj, myPoint3D deplacement) // return true si collision
{
	if ((me.minX + deplacement.X < obj.maxX)
		&& (me.maxX + deplacement.X > obj.minX)
		&& (me.minY < obj.maxY)
		&& (me.maxY > obj.minY)
		&& (me.minZ + deplacement.Z < obj.maxZ)
		&& (me.maxZ + deplacement.Z > obj.minZ))
	{
		return true;
	}

	return false;
}

bool collision() // return true si collision
{

	for each (myObject3D obj in objects)
	{
		if ((me.minX + deplacement().X < obj.maxX)
		&& (me.maxX + deplacement().X > obj.minX)
		&& (me.minY < obj.maxY) 
		&& (me.maxY > obj.minY)    
		&& (me.minZ + deplacement().Z < obj.maxZ)
		&& (me.maxZ + deplacement().Z > obj.minZ))
		{
			return true;
		}	
	}
	return false;
}

void computePos(int deltaMove) {	
	if (!collision())
	{
		camera_eye.X += deplacement().X;
		camera_eye.Z += deplacement().Z;

		me.translate(deplacement().X, 0, deplacement().Z);

		glutPostRedisplay();
	}
}

void computeDir(int deltaAngle) {
	camera_up.normalize();
	camera_forward.rotate(camera_up, 0.008 * deltaAngle);
	camera_forward.normalize();
	glutPostRedisplay();
}

void reshape(int width, int height){
	Glut_w = width;
	Glut_h = height;
	glm::mat4 projection_matrix = glm::perspective(fovy, Glut_w/(float)Glut_h, zNear, zFar);
	glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, &projection_matrix[0][0]);
	glViewport(0, 0, Glut_w, Glut_h);
}

//This function is called to display objects on screen.
void display() 
{
	if (deltaMove)
	{
		computePos(deltaMove);
	}	
	if (deltaAngle)
	{
		computeDir(deltaAngle);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, Glut_w, Glut_h);

	glm::mat4 projection_matrix = 
		glm::perspective(fovy, Glut_w/(float)Glut_h, zNear, zFar);
	glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, &projection_matrix[0][0]);

	glm::mat4 view_matrix = 
		glm::lookAt(glm::vec3(camera_eye.X, camera_eye.Y, camera_eye.Z), 
					glm::vec3(camera_eye.X + camera_forward.dX, camera_eye.Y + camera_forward.dY, camera_eye.Z + camera_forward.dZ), 
					glm::vec3(camera_up.dX, camera_up.dY, camera_up.dZ));
	glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, &view_matrix[0][0]);

	glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix)));
	glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, &normal_matrix[0][0]);

	glm::vec4 mylightPosition = glm::vec4(3,0,0,1);
	glUniform4fv(mylightPosition_loc, 1, &mylightPosition[0]);

	glm::vec4 mylightColor = glm::vec4(1,1,1,0);
	glUniform4fv(mylightColor_loc, 1,  &mylightColor[0]);

	glm::vec3 mylightDirection = glm::vec3(1,0,0);
	glUniform3fv(mylightDirection_loc, 1, &mylightDirection[0]);

	//int mylightType = 1;
	glUniform1i(mylightType_loc, mylightType);
	
	//obj->displayNormal();
	//me.displayObject(shaderprogram1, view_matrix);
	for each(myObject3D obj in objects)
	{
		obj.displayObject(shaderprogram1, view_matrix);
	}

	glPointSize(6.0);
	glBegin(GL_POINTS);
	glVertex3f(mylightPosition[0],mylightPosition[1],mylightPosition[2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(mylightPosition[0],mylightPosition[1],mylightPosition[2]);
	glVertex3f(mylightPosition[0]+mylightDirection[0],mylightPosition[1]+mylightDirection[1],mylightPosition[2]+mylightDirection[2]);
	glEnd();
	
	glFlush();
}

//This function is called from the main to initalize everything.
void init()
{
    vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl") ;
    fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl") ;
    shaderprogram1 = initprogram(vertexshader, fragmentshader);

	renderStyle_loc = glGetUniformLocation(shaderprogram1, "myrenderStyle") ;
	glUniform1i(renderStyle_loc, renderStyle);

	projection_matrix_loc = glGetUniformLocation(shaderprogram1, "myprojection_matrix");
	view_matrix_loc = glGetUniformLocation(shaderprogram1, "myview_matrix");
	normal_matrix_loc = glGetUniformLocation(shaderprogram1, "mynormal_matrix");

	mylightColor_loc = glGetUniformLocation(shaderprogram1, "mylightColor");
	mylightPosition_loc = glGetUniformLocation(shaderprogram1, "mylightPosition");
	mylightDirection_loc = glGetUniformLocation(shaderprogram1, "mylightDirection");
	mylightType_loc = glGetUniformLocation(shaderprogram1, "mylightType");
	
	me = myObject3D();
	me.readMesh("objects/me.obj");
	me.scale(0.5, 0.7, 0.5);
	me.translate(camera_eye.X, 0, camera_eye.Z);
	me.computeNormals();
	me.computeSphereTexture();
	me.computeTangents();
	me.createObjectBuffers();
	me.texture.readTexture("shingles-diffuse.ppm");
	me.bump.readTexture("shingles-normal.ppm");

	myObject3D *obj2, *obj3, *door;
	apple = new myObject3D();
	apple->readMesh("apple.obj");
	apple->translate(0, 4.1, 3);
	apple->scale(0.4, 0.4, 0.4);
	apple->computeNormals();
	apple->computeSphereTexture();
	apple->computeTangents();
	apple->createObjectBuffers();
	apple->texture.readTexture("objects/apple.ppm");
	objects.push_back(*apple);
	
	obj2 = new myObject3D();
	obj2->readMesh("table.obj");
	obj2->translate(0, 0, 0);
	obj2->computeNormals();
	obj2->computeSphereTexture();
	obj2->computeTangents();
	obj2->createObjectBuffers();
	obj2->texture.readTexture("objects/wood.ppm");
	objects.push_back(*obj2);

	obj3 = new myObject3D();
	obj3->readMesh("plane.obj");
	obj3->translate(0, 0, 0);
	obj3->scale(7, 10, 10);
	obj3->computeNormals();
	obj3->computeCylinderTexture();
	obj3->computeTangents();
	obj3->createObjectBuffers();
	obj3->texture.readTexture("shingles-diffuse.ppm");
	objects.push_back(*obj3);

	door = new myObject3D();
	door->readMesh("objects/door.obj");
	door->translate(0, 0, -5);
	door->computeNormals();
	door->computeSphereTexture();
	door->computeTangents();
	door->createObjectBuffers();
	door->texture.readTexture("objects/wood.ppm");
	objects.push_back(*door);

	myObject3D *wall1 = new myObject3D();
	wall1->readMesh("plane.obj");
	wall1->scale(3, 10, 10);
	wall1->translate(3, 7, 0);
	wall1->rotate(0,0,1,90);
	wall1->computeNormals();
	wall1->computeCylinderTexture();
	wall1->computeTangents();
	wall1->createObjectBuffers();
	wall1->texture.readTexture("shingles-diffuse.ppm");
	objects.push_back(*wall1);

	wall1 = new myObject3D();
	wall1->readMesh("plane.obj");
	wall1->scale(3, 10, 10);
	wall1->translate(3, -7, 0);
	wall1->rotate(0, 0, 1, 90);
	wall1->computeNormals();
	wall1->computeCylinderTexture();
	wall1->computeTangents();
	wall1->createObjectBuffers();
	wall1->texture.readTexture("shingles-diffuse.ppm");
	objects.push_back(*wall1);

	wall1 = new myObject3D();
	wall1->readMesh("plane.obj");
	wall1->scale(7, 3, 3);
	wall1->translate(0, -10, -3);
	wall1->rotate(1, 0, 0, 90);
	wall1->computeNormals();
	wall1->computeCylinderTexture();
	wall1->computeTangents();
	wall1->createObjectBuffers();
	wall1->texture.readTexture("shingles-diffuse.ppm");
	objects.push_back(*wall1);

	wall1 = new myObject3D();
	wall1->readMesh("plane.obj");
	wall1->scale(7, 3, 3);
	wall1->translate(0, 10, -3);
	wall1->rotate(1, 0, 0, 90);
	wall1->computeNormals();
	wall1->computeCylinderTexture();
	wall1->computeTangents();
	wall1->createObjectBuffers();
	wall1->texture.readTexture("shingles-diffuse.ppm");
	objects.push_back(*wall1);

	glUniform1i(glGetUniformLocation(shaderprogram1, "tex"), 1);
	glUniform1i(glGetUniformLocation(shaderprogram1, "bump"), 2);

	glClearColor(0.4, 0.4, 0.4, 0);
}


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutCreateWindow("My OpenGL Application");
	   
	glewInit() ; 
	glutReshapeWindow(Glut_w, Glut_h);
	
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	//glutKeyboardFunc(keyboard);
	//glutSpecialFunc(keyboard2);
	//glutMotionFunc(mousedrag);
	glutMotionFunc(mousedrag2);
	glutMouseFunc(mouse);
	//glutMouseWheelFunc(mouseWheel);
	glutKeyboardFunc(pressKey);
	glutIgnoreKeyRepeat(1);
	glutKeyboardUpFunc(releaseKey);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS) ;
	
	glEnable(GL_MULTISAMPLE);

	init();

	glutMainLoop();
	return 0;
}