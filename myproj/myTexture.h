#pragma once
#include <GL/glew.h>

class myTexture
{
public:
	int width, height, pixelsize;
	GLuint texName;

	GLubyte* myTexture::readFile(char *filename);
	void myTexture::readTexture(char *filename);
	void myTexture::cubeMapping(char *filename);
};