#include "respiration.h"

void Respiration::init()
{
	compileAndLinkShader();
	setLocations();
	allocateBuffers();
}
void Respiration::compileAndLinkShader()
{
	try {

		m_respirationProg.compileShader("shaders/respiration.cs");
		m_respirationProg.link();

	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

}
void Respiration::setLocations()
{

}

void Respiration::allocateBuffers()
{
	glGenBuffers(1, &m_bufferResp);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferResp);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 10 * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW); // 10 people max
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferResp);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	m_rollingWindow.resize(600, 0);
	m_SGWindowFast.resize(9, 0);
	m_SGSmoothYFast.resize(9, 0);
	m_SG1stDerivFast.resize(9, 0);

	m_smoothPlot.resize(600, 0);
	m_sgsPlot.resize(600, 0);
	m_sg1Plot.resize(600, 0);
}

void Respiration::setDepthTexture(GLuint depthTex)
{
	m_textureDepth = depthTex;
}

void Respiration::setFlowTexture(GLuint flowTex)
{
	m_textureFlow = flowTex;
}

void Respiration::setTarget(std::vector<glm::vec2> targetLoc)
{
	m_targetLocation = targetLoc;
}

void Respiration::calc()
{
	if (m_targetLocation.size() > 0)
	{
		m_respirationProg.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureDepth);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_textureFlow);

		std::vector<float> upData(4 * m_targetLocation.size(), 0);

		for (int target = 0; target < m_targetLocation.size(); target++)
		{
			upData[target * 4] = m_targetLocation[target].x;
			upData[target * 4 + 1] = m_targetLocation[target].y;
		}


		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferResp);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, upData.size() * sizeof(float), upData.data());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufferResp);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		int widthCompute = GLHelper::divup(m_targetLocation.size(), 8);

		glDispatchCompute(widthCompute, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);


		GLfloat *ptr;
		std::vector<float> outData(m_targetLocation.size() * 4, 0.0f);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferResp);
		ptr = (GLfloat *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		memcpy(outData.data(), ptr, m_targetLocation.size() * 4 * sizeof(float));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		//for (int i = 0; i < outData.size(); i++)
		//{
		//	std::cout << outData[i] << " ";
		//}
		//std::cout << std::endl;

		m_depth = outData[2];
		m_flow = outData[3];

		//neckDepth = outData[3] * outData[2];
		//smoothSignal(outData[2], 0);
	}
}
float Respiration::getFromDepth()
{
	return m_depth;
}

float Respiration::getFromFlow()
{
	return m_flow;
}

void Respiration::smoothSignal(float yData, std::string plotName)
{
	double ySum[600];
	double ySmooth[600];
	double sgs[600];
	double sg1[600];


	//float SGSmoothingCoeffWindowFast[5] = { -3, 12, 17, 12, -3 };
	float SGSmoothingCoeffWindowFast[9] = { -21,14,39,54,59,54,39,14,-21 };

	//float SG1stDerivCoeffWindowFast[5] = { -2, -1, 0, 1, 2 };
	float SG1stDerivCoeffWindowFast[9] = { -4,-3,-2,-1,0,1,2,3,4 };
	float SG2stDerivCoeffWindowFast[9] = { 28,7,-8,-17,-20,-17,-8,7,28 };

	m_rollingWindow.push_front(-1.0f*yData);
	m_rollingWindow.pop_back();

	float rollingAverage = 0.f;


	auto it5 = m_rollingWindow.begin();
	std::advance(it5, 25);

	for (std::list<float>::iterator it = m_rollingWindow.begin(); it != it5; it++)
	{
		rollingAverage += *it;
	}
	rollingAverage /= 25.0f; // there is probably a better way to get the mean... but i dont have wifi right now

	//std::cout << rollingAverage << std::endl;

	int yK = 0;
	for (std::list<float>::iterator it = m_rollingWindow.begin(); it != m_rollingWindow.end(); it++, yK++)
	{
		ySum[yK] = *it;
	}

	m_smoothPlot.push_front(rollingAverage);
	m_smoothPlot.pop_back();

	yK = 0;
	for (std::list<float>::iterator it = m_smoothPlot.begin(); it != m_smoothPlot.end(); it++, yK++)
	{
		ySmooth[yK] = *it;
	}

	m_SGWindowFast.push_front(rollingAverage);
	m_SGWindowFast.pop_back();

	float tempSGSmoothY = 0.0;
	float tempSG1stDeriv = 0.0f;
	int k = 0;
	for (std::list<float>::iterator it = m_SGWindowFast.begin(); it != m_SGWindowFast.end(); it++, k++)
	{
		tempSGSmoothY += SGSmoothingCoeffWindowFast[k] * (*it);
		tempSG1stDeriv += SG1stDerivCoeffWindowFast[k] * (*it);
	}



	m_SGSmoothYFast.push_front(tempSGSmoothY / 231.0);
	m_SG1stDerivFast.push_front(tempSG1stDeriv / 60.0);


	// Create iterator pointing to first element
	std::list<float>::iterator itList = m_SGSmoothYFast.begin();

	// Advance the iterator by 4 positions,
	std::advance(itList, 4);

	m_sgsPlot.push_front(*itList);
	m_sgsPlot.pop_back();

	yK = 0;
	for (std::list<float>::iterator it = m_sgsPlot.begin(); it != m_sgsPlot.end(); it++, yK++)
	{
		sgs[yK] = *it;
	}

	m_sgsPlotMat = cv::Mat(600, 1, CV_64F, sgs);
	cv::Mat plot_sgs;
	cv::Ptr<cv::plot::Plot2d> sgsPlot = cv::plot::Plot2d::create(m_sgsPlotMat);
	//sgsPlot->setMaxY(1.0);
	//sgsPlot->setMinY(-1.0);

	sgsPlot->render(plot_sgs);
	std::string imageName = "sgs " + plotName;

	cv::imshow(imageName, plot_sgs);








	if (m_meanBpm > 60.0f)
	{
		m_SGSmoothYFast.pop_front();
		m_SGSmoothYFast.push_front(rollingAverage);
	}

	m_SGSmoothYFast.pop_back();
	m_SG1stDerivFast.pop_back();
}

