#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/opencv.hpp>
#include <hikvision_api/utils.h>
#include <hikvision_api/timer.h>
#include <thread>

// Convert and publish the image
void publishImage(MV_FRAME_OUT *stImageInfo, image_transport::Publisher &image_pub, FrameInfo *pframe_info)
{
  // Step 1: Convert Hikvision image to OpenCV Mat
  cv::Mat img;
  if (stImageInfo->stFrameInfo.enPixelType == PixelType_Gvsp_BGR8_Packed)
  {
    // RGB8 format
    img = cv::Mat(stImageInfo->stFrameInfo.nHeight, stImageInfo->stFrameInfo.nWidth, CV_8UC3, stImageInfo->pBufAddr);
  }
  else if (stImageInfo->stFrameInfo.enPixelType == PixelType_Gvsp_Mono8)
  {
    // Mono8 format (grayscale)
    img = cv::Mat(stImageInfo->stFrameInfo.nHeight, stImageInfo->stFrameInfo.nWidth, CV_8UC1, stImageInfo->pBufAddr);
  }
  else
  {
    ROS_WARN("Unsupported pixel format!");
    return;
  }

  // Step 2: Convert OpenCV Mat to sensor_msgs/Image using cv_bridge
  sensor_msgs::ImagePtr msg;
  try
  {
    msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", img).toImageMsg();
  }
  catch (const cv_bridge::Exception &e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return;
  }

  const uint64_t timestamp_nano = pframe_info->getTimestampNano();
  const unsigned int frame_id = pframe_info->getFrameID();

  // Step 3: Populate the header with FrameInfo attributes
  msg->header.stamp = ros::Time(static_cast<uint32_t>(timestamp_nano / 1000000000), static_cast<uint32_t>(timestamp_nano % 1000000000));
  msg->header.seq = frame_id;                      // 注意：ROS的seq是uint32，可能需要根据实际情况调整
  msg->header.frame_id = std::to_string(frame_id); // 将frame_id转换为字符串

  // Step 3: Publish the image message
  image_pub.publish(msg);
}

// Thread function to continuously capture and publish images
void pop_thread(void *handle, image_transport::Publisher &image_pub)
{
  while (ros::ok())
  { // Ensure the thread stops when ROS shuts down
    auto frame = pop_image_buffer(handle, 1, false);
    if (frame != NULL)
    {
      // print_frame_info(frame, true);
      FrameInfo *frame_info = get_frame_info(&(frame->stFrameInfo));
      print_frame_info(frame, true);
      publishImage(frame, image_pub, frame_info);
      delete frame;
      delete frame_info;
    }
  }
}

// Camera work function
void camera_work(unsigned int idx, double freq, uint64_t sync_point, image_transport::Publisher &image_pub)
{
  Timer timer(sync_point, freq);

  // Initialize the camera using Hikvision SDK
  void *cam = init_SDK(idx);
  if (cam == NULL)
  {
    ROS_ERROR("Failed to initialize SDK.");
    return;
  }

  // Configure the camera
  set_exposure_auto_off(cam);
  set_exposure_time(cam, 15000.0);
  get_exposure_time(cam);
  set_pixel_format(cam, PixelType_Gvsp_BGR8_Packed);
  turn_on_IEEE1588(cam);
  print_IEEE1588_status(cam);
  set_trigger_mode_on(cam);
  set_trigger_source_to_action(cam);
  set_action_keys(cam);
  start_grabbing(cam);

  // Launch a thread to handle image capture and publishing
  std::thread capture_thread(pop_thread, cam, std::ref(image_pub));
  capture_thread.detach(); // Detach the thread to let it run independently

  // Synchronize and issue action commands
  timer.syncToFirstInterval();

  for (size_t i = 0; i < 100; i++) 
  {
    issue_action_command();
    timer.syncToNextInterval();
  }

  // Clean up and close the SDK
  stop_grabbing(cam);
  close_device(cam);
}

int main(int argc, char **argv)
{
  // Initialize the ROS node
  ros::init(argc, argv, "hikvision_image_publisher");
  ros::NodeHandle nh;

  // Define variables to store parameter values
  std::string image_topic = "";

  // Get parameters from the parameter server
  nh.param<std::string>("image_topic", image_topic, "/camera/image_raw");

  // Print the parameters to verify
  ROS_INFO("Image Topic: %s", image_topic.c_str());



  // Create an image publisher
  image_transport::ImageTransport it(nh);
  image_transport::Publisher image_pub = it.advertise(image_topic, 1);

  // Start camera work (launches a thread internally)
  ROS_INFO("Starting camera work...");
  camera_work(0, 20.0, 10000000, image_pub);

  // Keep the node alive
  ros::spin();

  return 0;
}