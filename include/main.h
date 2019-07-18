#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
#define GLM_ENABLE_EXPERIMENTAL
#include <stdio.h>
#include <iostream>
#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <deque>
#include <valarray>
#include <map>

//#include "openCVStuff.h"
#include "flow.h"
#include "flood.h"
#include "render.h"
#include "respiration.h"

#include "opencv2/core/utility.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/optflow.hpp"

#include "interface.h"
#include "openPoseWrapper.h"

#include <thread>
#include <mutex>
#include <sstream>




GLFWwindow *window;

gRender grender;

gFlow gflow;

gFlood gflood;

Respiration respiration;

//int trackedPerson::idCounter = -1;
//
//std::vector<trackedPerson> trackedPeople;

OPWrapper opwrapper;

Realsense2Interface cameraInterface;
std::vector<std::tuple<int, int, int, rs2_format>> depthProfiles;
std::vector<std::tuple<int, int, int, rs2_format>> colorProfiles;
std::vector<std::tuple<int, int, int, rs2_format>> infraProfiles;

std::vector<glm::ivec2> depthFrameSize;
std::vector<glm::ivec2> colorFrameSize;
std::vector<glm::ivec2> infraFrameSize;

cv::Mat newcol;

bool newFrameReady;

/////////////////////////
// KINECT STUFF

const int screenWidth = 1920;
const int screenHeight = 1080;

int colorWidth;
int colorHeight;

//const int depthWidth = 512;
//const int depthHeight = 424;

//float *mainColor[colorWidth * colorHeight];

//unsigned char colorArray[4 * colorWidth * colorHeight];

//float previousColorArray[depthWidth * depthHeight];
//float bigDepthArray[colorWidth * (colorHeight + 2)]; // 1082 is not a typo
													 //float color[512 * 424];
//float depthArray[depthWidth * depthHeight];
//float infraredArray[depthWidth * depthHeight];
//int colorDepthMap[depthWidth * depthHeight];

// depth color points picking
//bool select_color_points_mode = false;
//bool select_depth_points_mode = false;

//std::vector<cv::Point3f> depthPoints;
//std::vector<cv::Point2f> colorPoints;
//cv::Mat newColor;

bool showDepthFlag = false;
bool showBigDepthFlag = false;
bool showInfraFlag = false;
bool showColorFlag = true;
bool showLightFlag = false;
bool showPointFlag = true;
bool useOpenPoseFlag = false;
bool pauseOpenFlowFlag = false;
bool pauseFlowFlag = false;

bool showFlowFlag = true;
bool showEdgesFlag = false;
bool showNormalFlag = false;
bool showVolumeFlag = false;
bool showTrackFlag = false;
bool showFloodFlag = false;
bool showDistanceFlag = false;
bool showQuadsFlag = false;

float irBrightness = 1.0;
float irLow = 0.0f;
float irHigh = 65536.0f;
float vertFov = 40.0f;

float valA = 0.01f;
float valB = 0.01f;
int texLevel = 0;
int cutoff = 0;

bool useWebcamFlag = 1;
bool useImagesFlag = 0;
bool useVideosFlag = 0;

float xRot = 0.0f;
float zRot = 0.0f;
float yRot = 0.0f;
float xTran = 0.0f;
float yTran = 0.0f;
float zTran = 2000.0f;
void resetSliders() 
{
	vertFov = 40.0f;
	xRot = 0.0f;
	zRot = 0.0f;
	yRot = 0.0f;
	xTran = 0.0f;
	yTran = 0.0f;
	zTran = 2000.0f;
}

float zModelPC_offset = 0.0f;

//cv::Mat infraGrey;

bool calibratingFlag = false;

//////////////////////////////////////////////////
// SAVING IMAGES

// FUSION STUFF
bool trackDepthToPoint = true;
bool trackDepthToVolume = false;
int counter = 0;
bool reset = true;
bool integratingFlag = true;
bool selectInitialPoseFlag = false;

const char* sizes[] = { "32", "64", "128", "256", "384", "512", "768", "1024" };
static int sizeX = 2;
static int sizeY = 2;
static int sizeZ = 2;
float dimension = 1.0f;
float volSlice = 0.0f;

glm::vec3 iOff;

glm::vec3 initOffset(int pixX, int pixY)
{
	float z = 0;// depthArray[pixY * depthWidth + pixX] / 1000.0f;
	//kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()

	float x = 0;// (pixX - kcamera.ppx()) * (1.0f / kcamera.fx()) * z;
	float y = 0;// (pixY - kcamera.ppy()) * (1.0f / kcamera.fx()) * z;

	std::cout << "x " << x << " y " << y << " z " << z << std::endl;


	return glm::vec3(x, y, z);

}






// FLOW STUFF
//cv::VideoCapture cap;

std::vector<cv::Mat> imagesFromFile;
std::vector<cv::VideoCapture> videosFromFile;
int videoNumber = 0;
int videoFrameNumber = 0;

std::vector<std::pair<int, int> > resoPresetPair;
int resoPreset = 0;
int imageNumber = 0;

bool changedSource = false;

float mouseX = 0;
float mouseY = 0;


std::vector<cv::Mat> videoBuffer(100);

cv::Mat col;
cv::Mat infra;
std::mutex mtx;
std::thread *m_thread;
int m_status = 0;
bool m_newPoseFound = false;
bool m_wipeFlowFlag = false;
bool m_useOPFace = false;

bool m_justFlowFace = false;

bool useDelayFlag = false;
bool useFullResoFlag = true;

cv::VideoWriter outWriter;

float windowWidth = 10.0f;
// [person][frame][part]
std::map<int, std::deque<std::valarray<float>>> rollingAverage;


int desiredWidth = 848;
int desiredHeight = 480;
int desiredRate = 90;

int desiredColorWidth = 848;
int desiredColorHeight = 480;
int desiredColorRate = 60;

