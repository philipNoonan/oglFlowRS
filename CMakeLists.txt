cmake_minimum_required(VERSION 3.9)
# Toolchain file needs to be set before project() 
### FIND VCPKG
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
else()
	message(STATUS "VCPKG_ROOT not set in environment variables")	  
endif()


project(oglFlowRealsense)


if (WIN32)

if ("${CMAKE_VERSION}" VERSION_GREATER 3.6.3)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif ("${CMAKE_VERSION}" VERSION_GREATER 3.6.3)
  
endif()

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)




set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(BasicConfig)



find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(realsense2 REQUIRED)

find_package(OpenPose REQUIRED)
find_package(Caffe REQUIRED)
if (WIN32)
find_package(glog REQUIRED)
find_package(gFlags REQUIRED)
find_package(hdf5 REQUIRED)
find_package(protobuf REQUIRED)

# To find and use ffmpeg avcodec library
find_path( AVCODEC_INCLUDE_DIR libavcodec/avcodec.h )
find_library( AVCODEC_LIBRARY avcodec )

elseif (UNIX)
find_package(Glog REQUIRED)
find_package(GFlags REQUIRED)
endif()





file(GLOB_RECURSE SRC "src/*")
file(GLOB_RECURSE GFX "graphics/*")
file(GLOB_RECURSE SHD "shaders/*")
file(GLOB_RECURSE HDR "include/*")
file(GLOB_RECURSE RES "resources/*")
file(GLOB_RECURSE IMG "imgui/*")

list(APPEND SRC ${SHD})
list(APPEND SRC ${GFX})
list(APPEND SRC ${HDR})
list(APPEND SRC ${RES})
list(APPEND SRC ${IMG})

### COPY SHADER FILES TO BUILD AND BINARY LOCATIONS IF THEY HAVE BEEN CHANGED ###
add_custom_target(
  shaders ALL
  DEPENDS ${SHD}
  COMMENT "Copying shader files"
  VERBATIM
)
foreach(ShaderFile ${SHD})
get_filename_component(ShaderName ${ShaderFile} NAME)
	add_custom_command(
		TARGET shaders
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ShaderFile}
			${PROJECT_BINARY_DIR}/bin/$(Configuration)/shaders/${ShaderName}
	)
		add_custom_command(
		TARGET shaders
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ShaderFile}
			${PROJECT_BINARY_DIR}/shaders/${ShaderName}
	)
endforeach()

### COPY RESOURCES FILES TO BUILD AND BINARY LOCATIONS IF THEY HAVE BEEN CHANGED ###
add_custom_target(
  resources ALL
  DEPENDS ${RES}
  COMMENT "Copying resources files"
  VERBATIM
)
foreach(ResourceFile ${RES})
get_filename_component(ResourceName ${ResourceFile} NAME)
	add_custom_command(
		TARGET resources
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ResourceFile}
			${PROJECT_BINARY_DIR}/bin/$(Configuration)/resources/${ResourceName}
	)
		add_custom_command(
		TARGET resources
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_if_different
			${ResourceFile}
			${PROJECT_BINARY_DIR}/resources/${ResourceName}
	)
endforeach()


add_executable(${PROJECT_NAME} ${SRC})
add_dependencies(${PROJECT_NAME} shaders resources)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_PDB_NAME_DEBUG "${PROJECT_NAME}${CMAKE_DEBUG_POSTFIX}")

if (WIN32)
# FFMPEG
target_include_directories( ${PROJECT_NAME} PUBLIC  ${AVCODEC_INCLUDE_DIR} )
target_link_libraries( ${PROJECT_NAME}  ${AVCODEC_LIBRARY})
endif() # win32

target_include_directories(${PROJECT_NAME} PUBLIC "include")
target_include_directories(${PROJECT_NAME} PUBLIC "graphics")
target_include_directories(${PROJECT_NAME} PUBLIC "imgui")
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenPose_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)

target_link_libraries(${PROJECT_NAME} realsense2::realsense-file realsense2::realsense2)

if (WIN32)
target_link_libraries(${PROJECT_NAME} OpenPose::openpose 
					protobuf::libprotobuf
					hdf5::hdf5-shared
					gflags
					glog::glog
					caffe)
elseif (UNIX)
	  target_link_libraries(${PROJECT_NAME} ${OpenPose_LIBS} ${GFLAGS_LIBRARY} ${GLOG_LIBRARY} caffe)
endif()
