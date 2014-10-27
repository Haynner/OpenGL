#include "stdafx.h"
#include <fstream>
#include <GL/glew.h>
#include <freeglut.h>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <vector>
using namespace std;

#include "shaders.h"

#include "point3d.h"
#include "vector3d.h"
#include "myObject3D.h"

// width and height of the window.
int Glut_w = 600, Glut_h = 400; 

//Variables and their values for the camera setup.
myPoint3D camera_eye(0,0,4);
myVector3D camera_up(0,1,0);
myVector3D camera_forward (0,0,-1);

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


myObject3D *obj1,*obj2;

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
	case 's':
		renderStyle = (renderStyle+1)%3;
		glUniform1i(renderStyle_loc, renderStyle) ; 
		break;
	case 'r':
		camera_eye = myPoint3D(0,0,2);
		camera_up = myVector3D(0,1,0);
		camera_forward = myVector3D(0,0,-1);
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
		camera_eye += camera_forward*1.1;
		break;
	case GLUT_KEY_DOWN:
		camera_eye += -camera_forward*1.1;
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
	
	//obj1->displayNormal();
	obj1->displayObject(shaderprogram1,view_matrix);
	//obj2->displayObject(shaderprogram1,view_matrix);

	glPointSize(6.0);
	glBegin(GL_POINTS);
	glVertex3f(mylightPosition[0],mylightPosition[1],mylightPosition[2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(mylightPosition[0],mylightPosition[1],mylightPosition[2]);
	glVertex3f(mylightPosition[0]+mylightDirection[0],mylightPosition[1]+mylightDirection[1],mylightPosition[2]+mylightDirection[2]);
	glEnd();
	/*{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

	glDrawElements(GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, 0) ; 
	}*/

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
	

	obj1 = new myObject3D();
	obj1->readMesh("sphere.obj");
	obj1->computeNormals();
	obj1->computeSphereTexture();
	obj1->computeTangents();
	obj1->createObjectBuffers();
	obj1->texture.readTexture("shingles-diffuse.ppm");
	obj1->bump.readTexture("shingles-normal.ppm");
	//obj1->translate(1,1,1);
	
	

	/*obj2 = new myObject3D();
	obj2->readMesh("sphere.obj");
	obj2->computeNormals();
	obj2->computeSphereTexture();
	obj2->computeTangents();
	obj2->createObjectBuffers();
	obj2->texture.readTexture("earthmap.ppm",&obj1->texture.texName);
	obj1->texture.readTexture("earthmap.ppm",&obj1->texture.bumpName);*/
	//obj2->scale(1,1,0.5);

	glUniform1i(glGetUniformLocation(shaderprogram1, "tex"), 1);
	glUniform1i(glGetUniformLocation(shaderprogram1, "bump"), 2);


	/*{
	GLfloat verts[] = {1,1,1, 1,1,-1, 1,-1,1, 1,-1,-1, -1,1,1, -1,1,-1, -1,-1,1, -1,-1,-1};
	GLuint inds[] = {0,1,3, 0,3,2, 4,5,7, 4,7,6, 0,2,6, 0,6,4, 1,3,7, 1,7,5, 0,1,5, 0,5,4, 2,3,7, 2,7,6};
	vertices = vector<GLfloat> (verts, verts+24);
	indices = vector<GLuint> (inds, inds+36);

	glGenBuffers(2, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*4, &vertices.front(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*4, &indices.front(), GL_STATIC_DRAW);
	}*/


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
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard2);
	glutMotionFunc(mousedrag) ;
	glutMouseFunc(mouse) ;
	glutMouseWheelFunc(mouseWheel);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS) ;
	
	glEnable(GL_MULTISAMPLE);

	init();

	glutMainLoop();
	return 0;
}