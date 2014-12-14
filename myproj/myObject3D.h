#include <math.h>
#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <fstream>
#include "vector3d.h"
#include "myTexture.h";
#define PI 3.14159265

using namespace std;

class myObject3D
{
public:
	GLuint buffers[6];
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> textures;
	std::vector<GLfloat> tangents;

	glm::mat4 model_matrix;
	myTexture texture;
	myTexture cubemap;
	myTexture bump;

	float minX, maxX, minY, maxY, minZ, maxZ;

	myObject3D() {
		model_matrix = glm::mat4(1.0f);
		minX = 0;
		maxX = 0;
		minY = 0;
		maxY = 0;
		minZ = 0;
		maxZ = 0;
	}

	void clear() {
		vertices.clear();
		indices.clear();
		normals.clear();
		tangents.clear();
	}
 
	void readMesh(char *filename)
	{
		clear();
		string s, t;

		ifstream fin(filename);
		while ( getline(fin, s) )
		{
			stringstream myline(s);
			myline >> t;
			float vert;
			int ind;
			if (t == "v")
			{
				myline >> vert;
				vertices.push_back(vert);
				// research of x min and x max to the edge collision algo
				if (minX > vert)
				{
					minX = vert;
				}
				if (maxX < vert)
				{
					maxX = vert;
				}

				myline >> vert;
				vertices.push_back(vert);
				if (minY > vert)
				{
					minY = vert;
				}
				if (maxY < vert)
				{
					maxY = vert;
				}

				myline >> vert;
				vertices.push_back(vert);
				if (minZ > vert)
				{
					minZ = vert;
				}
				if (maxZ < vert)
				{
					maxZ = vert;
				}
			}
			else if (t == "f")
			{
				myline >> ind;
				indices.push_back(ind-1);
				myline >> ind;
				indices.push_back(ind-1);
				myline >> ind;
				indices.push_back(ind-1);
			}
		}
	}

	void computeNormal(int v1, int v2, int v3, float & x, float & y, float & z)
	{
		float ABx = vertices[v2*3]-vertices[v1*3];
		float ABy = vertices[v2*3 +1]-vertices[v1*3 +1];
		float ABz = vertices[v2*3 +2]-vertices[v1*3 +2];

		myVector3D AB(ABx,ABy,ABz);

		float BCx = vertices[v3*3]-vertices[v2*3];
		float BCy = vertices[v3*3 +1]-vertices[v2*3 +1];
		float BCz = vertices[v3*3 +2]-vertices[v2*3 +2];

		myVector3D BC(BCx,BCy,BCz);

		myVector3D res = AB.crossproduct(BC);
		res.normalize();
		//cout << res.dX <<endl;
		//cout << x <<endl;
		x = res.dX;
		y = res.dY;
		z = res.dZ;
		//cout << x <<endl;
		return;

	}

	void computeNormals( )
	{
		normals.resize(vertices.size());

		float x,y,z;
		for(int i=0;i<indices.size();i+=3)
		{
			computeNormal(indices[i],indices[i+1],indices[i+2],x,y,z);
			normals[indices[i]*3]+=x;
			normals[indices[i]*3+1]+=y;
			normals[indices[i]*3+2]+=z;

			normals[indices[i+1]*3]+=x;
			normals[indices[i+1]*3+1]+=y;
			normals[indices[i+1]*3+2]+=z;

			normals[indices[i+2]*3]+=x;
			normals[indices[i+2]*3+1]+=y;
			normals[indices[i+2]*3+2]+=z;
		}

		for(int k=0;k<normals.size();k+=3)
		{
			myVector3D v(normals[k],normals[k+1],normals[k+2]);
			v.normalize();
			normals[k]=v.dX;
			normals[k+1]=v.dY;
			normals[k+2]=v.dZ;

		}
		


	}

	void createObjectBuffers()
	{
		glGenBuffers(5, buffers);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*4, &vertices.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*4, &indices.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals.size()*4, &normals.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, textures.size()*4, &textures.front(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[4]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tangents.size()*4, &tangents.front(), GL_STATIC_DRAW);
	}

	void displayObject(GLuint shaderprogram, glm::mat4 viewmatrix)
	{
		glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mymodel_matrix"), 1, GL_FALSE, &model_matrix[0][0]);
		glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(viewmatrix*model_matrix)));
		glUniformMatrix3fv(glGetUniformLocation(shaderprogram, "mynormal_matrix"), 1, GL_FALSE, &normal_matrix[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture.texName);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, bump.texName);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.texName);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
		
		
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0) ; 
	}

	void displayNormal()
	{
		glBegin(GL_LINES);
		for (int i=0;i<vertices.size(); i+=3)
		{
			glVertex3f(vertices[i], vertices[i+1],vertices[i+2]);
			glVertex3f(vertices[i]+normals[i]/10.0, vertices[i+1]+normals[i+1]/10.0,vertices[i+2]+normals[i+2]/10.0);
		}
		glEnd();
	}

	void computeCylinderTexture()
	{
		int n =vertices.size()/3;
		textures.resize(2*n);
		GLfloat x, y, z;
		for (int i=0;i<n;i++)
		{
			x = vertices[3*i]; y = vertices[3*i+1]; z = vertices[3*i+2];
	
			textures[2*i] = z;
			if ( y>=0.0f )     textures[2*i+1] = atan2(  y,  x ) / (2*PI) ;
			else if ( y<0.0f )  textures[2*i+1] = (- atan2(  y,  x )) / (2*PI) ;
			//this has problems at the seam, when 1->0 and so interpoltion results in the whole image squeezed between the two border vertices.
			//if ( y>=0.0f )     textures[2*i+1] = atan2(  y,  x ) / (2*PI) ;
			//else if ( y<0.0f )  textures[2*i+1] = (2*PI + atan2(  y,  x )) / (2*PI) ;
		}
	}

	void computeSphereTexture()
	{
		int n =vertices.size()/3;
		textures.resize(2*n);
		GLfloat x, y, z;
		for (int i=0;i<n;i++)
		{
			x = vertices[3*i]; y = vertices[3*i+1]; z = vertices[3*i+2];
	
			textures[2*i+1] = acos(z/sqrt(z*z+y*y+x*x))/(PI);
			//if ( y>=0.0f )     textures[2*i+1] = atan2(  y,  x ) / (PI) ;
			//else if ( y<0.0f )  textures[2*i+1] = (- atan2(  y,  x )) / (PI) ;
			//this has problems at the seam, when 1->0 and so interpoltion results in the whole image squeezed between the two border vertices.
			if ( y>=0.0f )     textures[2*i] = atan2(  y,  x ) / (2*PI) ;
			else if ( y<0.0f )  textures[2*i] = (2*PI + atan2(  y,  x )) / (2*PI) ;
		}
	}

	void translate(double x, double y, double z)
	{
		glm::mat4 tmp = glm::translate(glm::vec3(x,y,z));
		model_matrix = tmp * model_matrix;
		minX += x;
		maxX += x;
		minZ += z;
		maxZ += z;
		minY += y;
		maxY += y;
	}

	void rotate(double axis_x, double axis_y, double axis_z, double angle)
	{
		glm::mat4 tmp = glm::rotate((float) angle, glm::vec3(axis_x, axis_y, axis_z));
		model_matrix = tmp * model_matrix;
	}

	void scale(double x, double y, double z)
	{
		glm::mat4 tmp = glm::scale(glm::vec3(x, y, z));
		model_matrix = tmp * model_matrix;
		minX *= x;
		maxX *= x;
		minZ *= z;
		maxZ *= z;
		minY *= y;
		maxY *= y;
	}

		void computeTangent(int v0, int v1, int v2, float & x, float & y, float & z)
	{
		float du1 = textures[2*v1] - textures[2*v0] ;
		float dv1 = textures[2*v1+1] - textures[2*v0+1] ;
		float du2 = textures[2*v2] - textures[2*v0] ;
		float dv2 = textures[2*v2+1] - textures[2*v0+1];

		float f = 1.0f / (du1 * dv2 - du2 * dv1);
		if ( (du1*dv2 - du2*dv1) == 0){
			x = y = z = 0; return;
		}

		float e1x = vertices[3*v1] - vertices[3*v0];
		float e1y = vertices[3*v1+1] - vertices[3*v0+1];
		float e1z = vertices[3*v1+2] - vertices[3*v0+2];

		float e2x = vertices[3*v2] - vertices[3*v0];
		float e2y = vertices[3*v2+1] - vertices[3*v0+1];
		float e2z = vertices[3*v2+2] - vertices[3*v0+2];

		x = f * ( dv2 * e1x - dv1 * e2x );
		y = f * ( dv2 * e1y - dv1 * e2y );
		z = f * ( dv2 * e1z - dv1 * e2z );
	}

	void computeTangents()
	{
		int i, j, k;
		GLfloat x1, y1, z1;

		int n = vertices.size()/3;
		int m = indices.size()/3;

		tangents.resize(3*n);
		int *incidences = new int[n];
		for (i=0;i<3*n;i++) tangents[i] = 0.0;
		for (i=0;i<n;i++) incidences[i] = 0;

		for (j=0;j<m;j++)
		{
			computeTangent(indices[3*j], indices[3*j+1], indices[3*j+2], x1, y1, z1);
			tangents[3*indices[3*j]] += x1; tangents[3*indices[3*j]+1] += y1; tangents[3*indices[3*j]+2] += z1;
			tangents[3*indices[3*j+1]] += x1; tangents[3*indices[3*j+1]+1] += y1; tangents[3*indices[3*j+1]+2] += z1;
			tangents[3*indices[3*j+2]] += x1; tangents[3*indices[3*j+2]+1] += y1; tangents[3*indices[3*j+2]+2] += z1;
			incidences[indices[3*j]]++; incidences[indices[3*j+1]]++; incidences[indices[3*j+2]]++;
		}
		for (i=0;i<n;i++) {
			float l = sqrt( tangents[3*i]*tangents[3*i] + tangents[3*i+1]*tangents[3*i+1] + tangents[3*i+2]*tangents[3*i+2] );
			tangents[3*i] /= l; tangents[3*i+1] /= l; tangents[3*i+2] /= l;
		}
	}
	
};
