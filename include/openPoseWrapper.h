#ifndef OP_WRAPPER_H
#define OP_WRAPPER_H

#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

// OpenPose dependencies
#include <openpose/headers.hpp>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


class OPWrapper
{

	enum Status
	{
		STOPPED = 0,
		CAPTURING = 1,
	};
public:
	OPWrapper() {};
	~OPWrapper() {};

	void start();
	void setImage(cv::Mat image);
	void stop();
	void setUseDelay(bool useDelay)
	{
		if (useDelay)
		{
			m_delay = 150;
		}
		else
		{
			m_delay = 0;
		}
	}
	void getPoses(cv::Mat &poses, std::vector<int> &poseIds)
	{
		poses = m_detectedKeyPointsPose;
		poseIds = m_detectedPoseIds;
	}


private:
	void capturingLoop();

	Status m_status = STOPPED;
	std::thread m_thread;
	std::mutex m_mtx;
	cv::Mat m_inputImage;
	cv::Mat m_detectedKeyPointsPose;
	std::vector<int> m_detectedPoseIds;
	int m_delay = 0;
};



#endif // OP_WRAPPER_H