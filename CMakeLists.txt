cmake_minimum_required(VERSION 2.8)
project( openCiVil )
message("Project source " ${PROJECT_SOURCE_DIR})
set(OpenCv_DIR "/usr/lib/opencv")
find_package( OpenCV REQUIRED core imgproc highgui videoio)
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable( openCiVil main.cpp marker.h marker.cpp GraphUtils.h )
target_link_libraries( openCiVil ${OpenCV_LIBS} )

