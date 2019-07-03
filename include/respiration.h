#ifndef RESPIRATION_H
#define RESPIRATION_H

#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glutils.h"
#include "glslprogram.h"

#include "glhelper.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include "opencv2/plot.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

class Respiration 
{
public:
	Respiration() {};
	~Respiration() {};

	void init();

	void setDepthTexture(GLuint depthTex);
	void setFlowTexture(GLuint flowTex);

	void setTarget(std::vector<glm::vec2> targetLoc);

	float getFromDepth();
	float getFromFlow();
	void calc();
	void smoothSignal(float yData, std::string plotName);

private:
	void compileAndLinkShader();
	void setLocations();
	void allocateBuffers();


	GLSLProgram m_respirationProg;

	GLuint m_bufferResp;
	GLuint m_textureDepth;
	GLuint m_textureFlow;

	std::vector<glm::vec2> m_targetLocation;

	cv::Mat m_sgsPlotMat;
	float m_meanBpm = 0.0f;

	float m_depth = 0.0f;
	float m_flow = 0.0f;

	std::list<float> m_rollingWindow;
	std::list<float> m_SGWindowFast;
	std::list<float> m_SGSmoothYFast;
	std::list<float> m_SG1stDerivFast;

	std::list<float> m_smoothPlot;
	std::list<float> m_sgsPlot;
	std::list<float> m_sg1Plot;

};






// Tracked Person
// we would like to be able to monitor each person uniquely
// int id;
// int chestSize;
// float breathingRate;
// int[2] screenPosition;
//
//class trackedPerson {
//public:
//	trackedPerson()
//	{
//		m_xNeck = 0;
//		m_yNeck = 0;
//
//		m_rollingWindow.resize(600, 0);
//		m_SGWindowFast.resize(9, 0);
//		m_SGSmoothYFast.resize(9, 0);
//		m_SG1stDerivFast.resize(9, 0);
//
//		m_smoothPlot.resize(600, 0);
//		m_sgsPlot.resize(600, 0);
//		m_sg1Plot.resize(600, 0);
//
//		m_ID = ++idCounter;
//	}
//	~trackedPerson()
//	{
//		idCounter--;
//	}
//	int ID()
//	{
//		return m_ID;
//	}
//	int chestSize()
//	{
//		return m_chestSize;
//	}
//	float breathingRate()
//	{
//		return m_breathingRate;
//	}
//	void checkCentroid(int x, int y)
//	{
//
//	}
//	void neckPos(int &x, int &y)
//	{
//		x = m_xNeck;
//		y = m_yNeck;
//	}
//	void setNeckPos(int x, int y)
//	{
//		m_xNeck = x;
//		m_yNeck = y;
//	}
//	std::list<float> rollingWindow()
//	{
//		return m_rollingWindow;
//	}
//	std::list<float> smoothPlot()
//	{
//		return m_smoothPlot;
//	}
//	std::list<float> SGWindowFast()
//	{
//		return m_SGWindowFast;
//	}
//	std::list<float> SGSmoothYFast()
//	{
//		return m_SGSmoothYFast;
//	}
//	std::list<float> SG1stDerivFast()
//	{
//		return m_SG1stDerivFast;
//	}
//	std::list<float> sgsPlot()
//	{
//		return m_sgsPlot;
//	}
//	std::list<float> m_rollingWindow;
//	std::list<float> m_SGWindowFast;
//	std::list<float> m_SGSmoothYFast;
//	std::list<float> m_SG1stDerivFast;
//
//	std::list<float> m_smoothPlot;
//	std::list<float> m_sgsPlot;
//	std::list<float> m_sg1Plot;
//
//private:
//	static int idCounter;
//	int m_ID;
//	int m_chestSize;
//	float m_breathingRate;
//	int m_xNeck;
//	int m_yNeck;
//
//
//
//	int screenPosition[2];
//	// use face recognition? may be difficult with occlusions
//	// use clothes colours? nurses/clinicans may be wearing similar clothes
//	// use screen space positon, and trajectories?
//	int getID()
//	{
//		return 0;
//	}
//	int getChestSize()
//	{
//		return 100;
//	}
//};
//














#endif // RESPIRATION_H
