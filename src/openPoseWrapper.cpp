#include "openPoseWrapper.h"



void OPWrapper::start()
{
	if (m_status == STOPPED)
	{
		m_status = CAPTURING;
		m_thread = std::thread(&OPWrapper::capturingLoop, this);

	}
}

void OPWrapper::setImage(cv::Mat image)
{
	//image.copyTo(m_inputImage);
	if (image.channels() == 1)
	{
		cv::cvtColor(image, m_inputImage, cv::COLOR_GRAY2RGB);
	}
	else if (image.channels() == 3)
	{
		cv::cvtColor(image, m_inputImage, cv::COLOR_BGR2RGB);
	}
	else if (image.channels() == 4)
	{
		cv::cvtColor(image, m_inputImage, cv::COLOR_BGRA2RGB);
	}


}

void OPWrapper::stop()
{
	
}

void OPWrapper::capturingLoop()
{
	op::Point<int> outputSize(-1, -1);
	op::Point<int> netInputSize(240, 240);

	op::Wrapper opWrapper{ op::ThreadManagerMode::Synchronous };
	op::PoseMode poseMode(op::PoseMode::Enabled);
	op::PoseModel poseModel(op::PoseModel::BODY_21A);

	op::ScaleMode scaleMode(op::ScaleMode::InputResolution);
	op::ScaleMode heatMapScaleMode(op::ScaleMode::UnsignedChar);
	int scaleNumber = 1;
	float scaleGap = 0.3f;

	op::ScaleAndSizeExtractor scaleAndSizeExtractor(netInputSize, outputSize, scaleNumber, scaleGap);

	op::PoseExtractorCaffeStaf poseExtractorCaffeStaf{ poseModel, "D://models//", 0 };
	op::PoseCpuRenderer poseRenderer{ poseModel, 0.05f, true, 0.05f };

	op::CvMatToOpInput cvMatToOpInput;
	op::CvMatToOpOutput cvMatToOpOutput;
	op::OpOutputToCvMat opOutputToCvMat;


	poseExtractorCaffeStaf.initializationOnThread();
	poseRenderer.initializationOnThread();

	op::GuiInfoAdder guiInfoAdder{ 1, true };



	while (m_status == CAPTURING)
	{
		
		const op::Point<int> imageSize{ m_inputImage.cols, m_inputImage.rows };
		std::vector<double> scaleInputToNetInputs;
		std::vector<op::Point<int>> netInputSizes;

		double scaleInputToOutput;
		op::Point<int> outputResolution;

		std::tie(scaleInputToNetInputs, netInputSizes, scaleInputToOutput, outputResolution) = scaleAndSizeExtractor.extract(imageSize);

		const auto netInputArray = cvMatToOpInput.createArray(m_inputImage, scaleInputToNetInputs, netInputSizes);
		auto outputArray = cvMatToOpOutput.createArray(m_inputImage, scaleInputToOutput, outputResolution);

		poseExtractorCaffeStaf.forwardPass(netInputArray, imageSize, scaleInputToNetInputs);

		const auto poseKeypoints = poseExtractorCaffeStaf.getPoseKeypoints();
		const auto poseIds = poseExtractorCaffeStaf.getPoseIds();

		auto poses = poseExtractorCaffeStaf.getPoseKeypoints().clone();

		m_mtx.lock();
		m_detectedKeyPointsPose = poses.getConstCvMat().clone();
		m_mtx.unlock();

		//poseRenderer.renderPose(outputArray, poseKeypoints, scaleInputToOutput, -1.0f, poseIds);

		// Step 5 - Render poseKeypoints
		// poseRenderer.renderPose(outputArray, poseKeypoints, scaleInputToOutput);
		// Step 6 - OpenPose output format to cv::Mat
		//auto outputImage = opOutputToCvMat.formatToCvMat(outputArray);

		//guiInfoAdder.addInfo(outputImage, 69, 8008, "sup bros", 8008135, poseIds, poseKeypoints);
		
		//cv::imshow("points", outputImage);

		//cv::waitKey(1);

		{
			//using namespace std::chrono_literals;
			//std::this_thread::sleep_for(0.15s);

		}
	}

}