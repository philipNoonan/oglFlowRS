# oglFlowRealsense
An openGL GLSL implementation of Dense Inverse Search Optical Flow with added STAF Openpose body tracking 

![Alt text](docs/oglflowRS.jpg?raw=true "Title")

<h2>Installation</h2>

<h3>Dependencies</h3>

We use vcpkg to install dependencies. Get vcpkg from the link and follow its installation instructions.

<a href="https://github.com/Microsoft/vcpkg">VCPKG</a> 

If you dont want to use vcpkg, then just ensure that the listed 3rd party libs are cmake findable on your system.

<h4>Windows</h4>

To make vcpkg use a little cleaner we set two environment variables, defining the tpe of system (x64 / x86) and the location of vcpkg.exe. Open a command promt with administrator privilages (hit windows key, type "cmd", right click "Command Prompt" and choose "Run as Administrator") .
These commands may take a few seconds to execute.

```
setx VCPKG_DEFAULT_TRIPLET "x64-windows" /m
setx VCPKG_ROOT "C:\vcpkg" /m
```
Close the Admin Command Prompt window to flush the newly set variables.

Go to your vcpkg.exe installed location and open another command prompt.



```
vcpkg install glew glfw3 glm imgui realsense2 opencv
```
This should take 3-4 minutes.

<h3> REQUIRED FOR GETTING TIMESTAMPS ON WINDOWS </h3>
To get timestamps from the realsense camera itself, rather than the time at which the host computer receives the frame we need to follow the steps described in <a href="https://github.com/IntelRealSense/librealsense/blob/c3c758d18c585a237bb5b635927797aa69996391/doc/installation_windows.md">these intel instructions</a> under the section labeled "Enabling metadata on Windows"

<h3> Installing oglFlowRealsense </h3>

We use <a href="https://www.visualstudio.com/downloads/">visual studio 2017</a> since it is the most readily available MSVC these days, support for c++17 features, and the hope that it will be useable with cuda 9.2.

We use <a href="https://cmake.org/download/">cmake</a> . Please use the latest version available.

Pull the latest version of oglFlowRealsense

```
git clone https://github.com/philipNoonan/oglFlowRS
```

Open CMake and set the source directory as "PATH_TO_YOUR_VERSION/oglFusionRealsense/" and the build directory as "PATH_TO_YOUR_VERSION/oglFusionRealsense/build"

Choose to create a new folder, and choose MSVC 15 2017 x64 as the generator.

Press "Configure"

Press "Generate"

Press "Open Project"

<h3>Installing on Ubuntu and/or the Jetson Nano</h3>

Easy bits:

sudo apt-get install libglew-dev 

sudo apt-get install libglfw3
sudo apt-get install libglfw3-dev

sudo apt-get install libglm-dev

Less easy bits:

The sdk for the realsense can be downloaded and installed from <a href="https://github.com/IntelRealSense/librealsense">source</a>  

OpenCV > 4.0 should also be installed from <a href="https://github.com/opencv/opencv ">source</a>

The STAF version of openpose is currently on this <a href="https://github.com/soulslicer/openpose/tree/staf ">fork</a> of the CMU openpose repo. The installation is the same as the openpose repo, but note that you should checkout the STAF branch.

