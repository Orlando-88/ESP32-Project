# 放在 D:\AutoRubbishRobot\Orlando\CMakeLists.txt
# 这是告诉 ESP-IDF 这是一个独立的项目，项目名字叫 Orlando_Robot

cmake_minimum_required(VERSION 3.16)

# 包含 ESP-IDF 的标准构建配置
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 定义你的项目名称
project(Orlando_Robot)