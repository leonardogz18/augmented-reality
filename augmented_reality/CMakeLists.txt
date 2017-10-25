cmake_minimum_required(VERSION 2.8)
SET(CMAKE_CXX_FLAGS "-std=c++11")
project( Points_detection )

find_package( OpenCV REQUIRED )
#OPENGL 
find_package(OpenGL REQUIRED)

add_executable( coordinate_system coordinate_system.cpp functions.h cali/calibrator.h 
	cali/functions.h 
	)


target_link_libraries( coordinate_system ${OpenCV_LIBS} -lGLEW -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl -lglut -lGLU)


SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )

add_custom_command(
   TARGET coordinate_system POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/"
)

