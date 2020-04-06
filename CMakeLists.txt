CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
PROJECT( edge_detection )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

SET(OpenCV_DIR /home/jhchenjh/opencv_3.4.4/share/OpenCV/)
find_package(OpenCV REQUIRED QUIET)

link_directories(
  ${catkin_INCLUDE_DIRS}
	${OpenCV_LIBRARY_DIRS}
)

add_executable(canny src/canny.cpp)
target_link_libraries(canny ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_executable(log src/LoG.cpp)
target_link_libraries(log ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_library(edge_devernay src/edge_devernay.cpp src/edge_devernay.h)
target_link_libraries(edge_devernay ${OpenCV_LIBRARIES})

add_executable(canny_devernay src/canny_devernay_subpixel.cpp)
target_link_libraries(canny_devernay edge_devernay ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_executable(raw_to_pgm src/raw_to_pgm.cpp)
target_link_libraries(raw_to_pgm ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})
