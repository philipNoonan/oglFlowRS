#include "main.h"



static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

void gRenderInit()
{
	grender.compileAndLinkShader();
	grender.setColorSize(colorFrameSize[0].x, colorFrameSize[0].y);

	grender.setBuffers(gflow.getQuadlist(), gflow.getQuadlistMeanTemp());

	grender.setLocations();
	grender.setVertPositions();
	grender.allocateBuffers();
	grender.allocateTextures();
	grender.setTextures(gflow.getDepthTexture(), gflow.getColorTexture(), gflow.getEdgesTexture(), gflow.getFlowMinusMeanFlowTexture()); //needs texture uints from gfusion init
//krender.genTexCoordOffsets(1, 1, 1.0f);
}

void preLoadVideo(int vidNumber)
{
	int vidWidth = videosFromFile[vidNumber].get(cv::CAP_PROP_FRAME_WIDTH);
	int vidHeight = videosFromFile[vidNumber].get(cv::CAP_PROP_FRAME_HEIGHT);

	int frameNumber = 0;
	videoBuffer.resize(videosFromFile[vidNumber].get(cv::CAP_PROP_FRAME_COUNT) - 50);


	while (frameNumber < videoBuffer.size())
	{
		videosFromFile[vidNumber].read(videoBuffer[frameNumber]);
		frameNumber++;
	}


}


void searchForMedia()
{
	videosFromFile.resize(0);
	imagesFromFile.resize(0);

	//cv::String pathVideos("videos/*.avi"); //select only wmv
	cv::String pathVideos("videos/*.wmv"); //select only wmv

	//cv::String pathVideos("videos/*.mkv"); //select only wmv
	//cv::String pathVideos("videos/*.mp4"); //select only mkv
	//cv::String pathVideos("videos/*.MP4"); //select only mkv

	std::vector<cv::String> fnVideos;
	cv::glob(pathVideos, fnVideos, true); // recurse



	for (size_t k = 0; k<fnVideos.size(); ++k) 
	{
		std::cout << fnVideos[k] << std::endl;

		cv::VideoCapture cap(fnVideos[k]);
		cap.set(cv::CAP_PROP_BUFFERSIZE, 5);
		if (!cap.isOpened())
		{
			std::cout << "cannot open video file" << std::endl;
			//return;
		}

		videosFromFile.push_back(cap);
	}

	outWriter.open("output/outputWiM.wmv", static_cast<int>(videosFromFile[0].get(cv::CAP_PROP_FOURCC)), 30, cv::Size(806, 540), true);
	if (!outWriter.isOpened())
	{
		std::cout << "Could not open the output video for write" << std::endl;
		//return -1;
	}

	cv::String pathImages("images/*.png"); //select only jpg
	std::vector<cv::String> fnImages;
	cv::glob(pathImages, fnImages, true); // recurse
	for (size_t k = 0; k<fnImages.size(); ++k)
	{
		std::cout << fnImages[k] << std::endl;

		cv::Mat im = cv::imread(fnImages[k]);
		if (im.empty())
		{
			std::cout << "empty image " << std::endl;
			continue; //only proceed if sucsessful
		}					  // you probably want to do some preprocessing
							  //if (k == 0)
							  //imagesFromFile.push_back(cv::Mat(im.rows, im.cols, CV_8UC3));
		imagesFromFile.push_back(im);
	}
}


void resetFlowSize()
{

	int numberOfCameras = cameraInterface.searchForCameras();

	if (numberOfCameras > 0)
	{
		depthProfiles.resize(numberOfCameras, 71); // 71 on 435, 211 on 415
		colorProfiles.resize(numberOfCameras, 61); // 0 1920x1080x30 rgb8 // 13 1920x1080x6 rgb8
		infraProfiles.resize(numberOfCameras, 15); // 15 on 435 35 on 415
		depthFrameSize.resize(numberOfCameras);
		colorFrameSize.resize(numberOfCameras);
		infraFrameSize.resize(numberOfCameras);

		for (int camera = 0; camera < numberOfCameras; camera++)
		{
			cameraInterface.startDevice(camera, depthProfiles[camera], colorProfiles[camera]);
			cameraInterface.setDepthTable(camera, 50000, 0, 100, 0, 0);
			cameraInterface.setEmitterOptions(camera, false, 100.0f);

			int wd, hd, rd;
			int wc, hc, rc;
			cameraInterface.getDepthProperties(camera, wd, hd, rd);
			cameraInterface.getColorProperties(camera, wc, hc, rc);

			//colorToDepth[camera] = cameraInterface.getColorToDepthExtrinsics(camera);
			//depthToColor[camera] = cameraInterface.getDepthToColorExtrinsics(camera);

			depthFrameSize[camera].x = wd;
			depthFrameSize[camera].y = hd;

			infraFrameSize[camera].x = wd;
			infraFrameSize[camera].y = hd;

			colorFrameSize[camera].x = wc;
			colorFrameSize[camera].y = hc;
		}

	}

	gflow.firstFrame = true;
	//gflow.clearTexturesAndBuffers();
	gflow.setNumLevels(colorFrameSize[0].x);
	gflow.setTextureParameters(colorFrameSize[0].x, colorFrameSize[0].y);
	gflow.allocateTextures(1);
	gflow.allocateBuffers();
	gflow.allocateOffscreenRendering();

	

	changedSource = false;
}

void showImagePairs()
{





}


int main(int, char**)
{


	int display_w, display_h;
	// load openGL window
	window = grender.loadGLFWWindow();

	glfwGetFramebufferSize(window, &display_w, &display_h);
	// Setup ImGui binding
	ImGui::CreateContext();

	ImGui_ImplGlfwGL3_Init(window, true);
	ImVec4 clear_color = ImColor(114, 144, 154);

	resoPresetPair.push_back(std::make_pair(640, 480));
	resoPresetPair.push_back(std::make_pair(960, 540));
	resoPresetPair.push_back(std::make_pair(1280, 720));
	resoPresetPair.push_back(std::make_pair(1920, 1080));

	colorWidth = resoPresetPair[resoPreset].first;
	colorHeight = resoPresetPair[resoPreset].second;

	//gflow.setupEKF();
	// op flow init
	gflow.compileAndLinkShader();
	gflow.setLocations();


	resetFlowSize();
	gRenderInit();

	respiration.init();
	respiration.setDepthTexture(gflow.getDepthTexture());
	respiration.setFlowTexture(gflow.getFlowTexture());

	//gflow.setNumLevels(colorFrameSize[0].x);
	//gflow.setTextureParameters(colorFrameSize[0].x, colorFrameSize[0].y);
	//gflow.allocateTextures(col.channels());

	//gflow.allocateBuffers();
	//gflow.allocateOffscreenRendering();


	gflood.compileAndLinkShader();
	gflood.setLocations();

	gflood.setTextureParameters(colorFrameSize[0].x, colorFrameSize[0].y); // rename me to texture width and height
	gflood.allocateTextures();
	gflood.allocateBuffers();

	opwrapper.start();

	//rollingAverage.resize(windowWidth);
    // [person][frame][part]
	// rollingAverage.resize(15, std::deque<std::valarray<float>> (windowWidth, std::valarray<float>(63)));




	double lastTime = glfwGetTime();
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &display_w, &display_h);
		
		//// Rendering
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		bool frameReady = cameraInterface.collateFrames();

		if (frameReady)
		{

			gflow.setDepthTexture(cameraInterface.getDepthQueues());
			gflow.setInfraTexture(cameraInterface.getInfraQueues(), infra);
			gflow.setColorTexture(cameraInterface.getColorQueues(), col);

			//cv::imshow("cols", col);
			//cv::waitKey(1);

			if (!infra.empty())
			{
				opwrapper.setImage(infra);
			}

			gflow.calc(true);
			//gflow.track();
			if (showQuadsFlag)
			{
				gflow.buildQuadtree();
				grender.setQuadlistCount(gflow.getQuadlistCount());

			}

			grender.setFlowTexture(gflow.getFlowTexture());

			//gflood.setFloodInitialRGBTexture(col.data, colorFrameSize[0].x, colorFrameSize[0].y, 3);


			if (showDistanceFlag)
			{
				gflood.jumpFloodCalc();
				grender.setDistanceTexture(gflood.getFloodOutputTexture());
			}

		}
			
		//std::cout << opwrapper.getPoses() << std::endl;
		cv::Mat poses;
		std::vector<int> poseIds;
		opwrapper.getPoses(poses, poseIds);

		if (!poses.empty())
		{
			cv::Size poseSize = poses.size();

			std::vector<std::valarray<float>> bpp(poseSize.height, std::valarray<float>(poseSize.width * 3));
			std::vector<std::valarray<float>> RA(poseSize.height, std::valarray<float>(poseSize.width * 3));
			std::vector<glm::vec2> neckPos(poseSize.height);

			for (int person = 0; person < poseSize.height; person++)
			{
				//std::cout << "id : " << poseIds[person] << std::endl;
				for (int part = 0; part < poseSize.width; part++)
				{
					if (poses.at<cv::Vec3f>(person, part)[2] > 0)
					{
						bpp[person][part * 3] = poses.at<cv::Vec3f>(person, part)[0];
						bpp[person][part * 3 + 1] = poses.at<cv::Vec3f>(person, part)[1];
						bpp[person][part * 3 + 2] = poses.at<cv::Vec3f>(person, part)[2];
					}
					else // get the last frames position
					{
						auto it = rollingAverage.find(poseIds[person]);
						if (it != rollingAverage.end())
						{
							bpp[person][part * 3] = it->second[0][part * 3];
							bpp[person][part * 3 + 1] = it->second[0][part * 3 + 1];
							bpp[person][part * 3 + 2] = 0; // reduce the weighting of these points
						}
						else
						{
							bpp[person][part * 3] = 0;
							bpp[person][part * 3 + 1] = 0;
							bpp[person][part * 3 + 2] = 0;
						}

					}

					//std::cout << "x : " << x << " y : " << y << std::endl;

				}

				auto it = rollingAverage.find(poseIds[person]);
				// person already in map
				if (it != rollingAverage.end())
				{
					it->second.push_front(bpp[person]);
					if (it->second.size() > windowWidth)
					{
						it->second.pop_back();
					}

					for (std::deque<std::valarray<float>>::iterator dit = it->second.begin(); dit != it->second.end(); dit++)
					{
						RA[person] += *dit;
					}

					RA[person] /= windowWidth;

					neckPos[person] = glm::vec2(RA[person][1 * 3], RA[person][1 * 3 + 1] + 50.0f);

					
				}
				else // add person to map
				{
					std::deque<std::valarray<float>> tempbpp(windowWidth, bpp[person]);
					rollingAverage.insert(std::pair<int, std::deque<std::valarray<float>>> (poseIds[person], tempbpp));
				}
				//rollingAverage[poseIDs[person]].push_front(bpp[person]);

				// [person][frame][part]

			}

			grender.setBodyPosePoints(RA);
			respiration.setTarget(neckPos);

		}

		respiration.calc();

		float neckDepth = respiration.getFromDepth();
		float neckFlow = respiration.getFromFlow();

		//if (neckDepth != 0.0f)
		//{
		//	respiration.smoothSignal(neckDepth, "depth");
		//}

		if (neckFlow != 0.0f)
		{
			respiration.smoothSignal(neckFlow, "flow");
		}

			glfwPollEvents();
			ImGui_ImplGlfwGL3_NewFrame();

			grender.setRenderingOptions(showDepthFlag, showBigDepthFlag, showInfraFlag, showColorFlag, showLightFlag, showPointFlag, showFlowFlag, showEdgesFlag, showNormalFlag, showVolumeFlag, showTrackFlag, showDistanceFlag, showQuadsFlag);

				grender.setColorImageRenderPosition(vertFov);

				grender.setFlowImageRenderPosition(colorFrameSize[0].x, colorFrameSize[0].y, vertFov);

				grender.setViewMatrix(xRot, yRot, zRot, xTran, yTran, zTran);
				grender.setProjectionMatrix();

				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);

			grender.Render(true);

			if (grender.showImgui())
			{
				//ImGui::SetNextWindowPos(ImVec2(1600 - 32 - 528 - 150, 32));
				//ImGui::SetNextWindowSize(ImVec2(528 + 150, 424), ImGuiSetCond_Always);
				ImGuiWindowFlags window_flags = 0;

				float arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				arr[0] = gflow.getTimeElapsed();
				arr[8] = arr[0];// +arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];
				GLint total_mem_kb = 0;
				glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
					&total_mem_kb);

				GLint cur_avail_mem_kb = 0;
				glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
					&cur_avail_mem_kb);

				bool showGUI = grender.showImgui();
				ImGui::Begin("Menu", &showGUI, window_flags);
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);
				ImGui::Text("GPU Memory Usage %d MB out of %d (%.1f %%)", (total_mem_kb - cur_avail_mem_kb) / 1024, total_mem_kb / 1024, 100.0f * (1.0f - (float)cur_avail_mem_kb / (float)total_mem_kb));




				//ImGui::PushItemWidth(-krender.guiPadding().first);
				//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
				ImGui::Text("Help menu - press 'H' to hide");
				ImGui::Separator();
				
				ImGui::Separator();
				ImGui::Text("View Options");

				if (ImGui::Button("Show Color")) showColorFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showColorFlag); ImGui::SameLine(); if (ImGui::Button("Show Depth")) showDepthFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showDepthFlag);

				
				if (ImGui::Button("Show Flow")) showFlowFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFlowFlag); ImGui::SameLine(); if (ImGui::Button("Show Edges")) showEdgesFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showEdgesFlag);
				
				if (ImGui::Button("Show Point")) showPointFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showPointFlag);

				//if (ImGui::Button("Show flood")) showFloodFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFloodFlag);
				if (ImGui::Button("Show flood")) showDistanceFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showDistanceFlag); ImGui::SameLine(); if (ImGui::Button("Show Quads")) showQuadsFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showQuadsFlag);

				ImGui::Separator();
				ImGui::Text("Other Options");

				if (ImGui::Button("Reset flow points")) gflow.clearPoints();
				//if (ImGui::Button("Reset")) OCVStuff.resetColorPoints();

				//if (ImGui::Button("Reset Depth")) krender.resetRegistrationMatrix();

				//if (ImGui::Button("Export PLY")) krender.setExportPly(true);
				//if (ImGui::Button("Export PLY")) krender.exportPointCloud();
				//if (ImGui::Button("Save Color")) OCVStuff.saveImage(0); // saving color image (flag == 0)


				ImGui::Separator();
				ImGui::Text("View Transforms");
				ImGui::SliderFloat("vFOV", &vertFov, 1.0f, 90.0f);
				ImGui::SliderInt("tex level", &texLevel, 0, 9);
				ImGui::SliderFloat("valA", &valA, 0.1f, 50.0f);
				ImGui::SliderFloat("valB", &valB, 0.00001f, 0.5f);
				ImGui::SliderInt("cutoff", &cutoff, 0, 7);

				grender.setFov(vertFov);
				gflow.setVals(valA, valB);
				gflow.setCutoff(cutoff);
				gflood.setEdgeThreshold(valA);
				grender.setRenderLevel(texLevel);

				if (ImGui::Button("Reset Sliders")) resetSliders();




				ImGui::End();

			}




			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

			if (changedSource)
			{
				// if use video
				// do soemthign 
				// if use webcam
				// do somethign else
				resetFlowSize();
			}


			//grender.setComputeWindowPosition();
			//gfusion.render();
			glfwSwapBuffers(window);
		}

	

	// Cleanup DO SOME CLEANING!!!
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();


	//krender.cleanUp();

	return 0;
}