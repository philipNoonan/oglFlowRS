#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glutils.h"
#include "glslprogram.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

class gFlood
{
public:
	gFlood() {};
	~gFlood() {};
	
	void compileAndLinkShader();
	void setLocations();
	void allocateBuffers();
	void allocateTextures();

	void setTextureParameters(int width, int height) { m_texture_width = width;  m_texture_height = height; }
	
	void pushBackTP(float x, float y);

	void wipeFlood();
	void clearPoints();

	
	void jumpFloodCalc();


	double getTimeElapsed()
	{
		return m_timeElapsed;
	}

	GLuint getFloodOutputTexture()
	{
		return m_texture_output_RGBA;
	}
	GLuint getFloodInitialTexture()
	{
		return m_texture_initial;
	}
	void setFloodInitialRGBTexture(unsigned char * data, int width, int height, int nrChan);
	GLuint getFloodInitialRGBTexture()
	{
		return m_texture_initial_RGB;
	}
	void setEdgeThreshold(float thresh)
	{
		m_edgeThreshold = thresh;
	}


private:

	GLuint timeQuery[1];
	double m_timeElapsed = 0.0;

	inline int divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }

	GLSLProgram jumpFloodProg;
	GLSLProgram edgeDetectProg;

	//Locations
	/* subroutines */
	GLuint m_subroutine_jumpFloodID;
	GLuint m_subroutine_edgeDetectID;

	/* uniforms */
	GLuint m_jfaInitID;
	GLuint m_jfaUpdateID;
	GLuint m_jfaFastUpdateID;
	GLuint m_jumpID;
	GLuint m_applyFilterID;
	GLuint m_edgeThresholdID;
	GLuint m_getColorID;

	//Buffers
	GLuint m_bufferClickedPoints;



	//Textures
	GLuint createTexture(GLuint ID, GLenum target, int levels, int w, int h, int d, GLuint internalformat);

	//JFA
	GLuint m_texture_initial;
	GLuint m_texture_jfa_0;
	GLuint m_texture_jfa_1;
	GLuint m_texture_initial_RGB;
	GLuint m_texture_output_RGBA;

	// parameters
	int m_texture_width;
	int m_texture_height;

	std::vector<float> m_trackedPoints;

	float m_edgeThreshold = 0.1f;

	void uploadTP();


};