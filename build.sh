#!/bin/bash

ROOT=$(pwd)

# 检查是否有 MVCAM_COMMON_RUNENV 环境变量
if [ -z "$MVCAM_COMMON_RUNENV" ]; then
  echo "Error: Environment variable MVCAM_COMMON_RUNENV is not set."
  echo "You need to install MVS first."
  exit 1
else
  echo "MVCAM_COMMON_RUNENV is set to: $MVCAM_COMMON_RUNENV"
fi


# 检查是否存在 build 文件夹
if [ ! -d "build" ]; then
  echo "build folder does not exist. Start building..."
  mkdir build
  cd build
  git clone https://github.com/leoxiang66/hikvision_camera_api.git
  cd hikvision_camera_api
  sudo make install
  cd $ROOT
fi