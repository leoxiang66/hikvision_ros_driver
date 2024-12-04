cmake_minimum_required(VERSION 3.0.2)
project(hikvision_ros_driver)

add_compile_options(-std=c++11)

set(catkin_DIR "/opt/ros/noetic/share/catkin/cmake")

find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  image_transport
  roscpp
  sensor_msgs
  std_msgs
)


find_library(HIKVISION_API_LIBRARY hikvision_api PATHS /usr/local/lib)
find_library(MVCAMERA_LIBRARY MvCameraControl PATHS $ENV{MVCAM_COMMON_RUNENV}/64)


catkin_package()


include_directories(
# include
  ${catkin_INCLUDE_DIRS}
  /usr/local/include
)


add_executable(image_publisher_node src/image_publisher_node.cpp)

target_link_libraries(image_publisher_node
  ${catkin_LIBRARIES}
  ${HIKVISION_API_LIBRARY}
  ${MVCAMERA_LIBRARY}
  pthread
)



