cmake_minimum_required(VERSION 3.10)
project(ReadTime)

# 查找 ROOT
find_package(ROOT REQUIRED COMPONENTS Tree)

# 包含 ROOT 头文件和库
include(${ROOT_USE_FILE})

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(check_time read_time.cpp)

# 链接 ROOT 库
target_link_libraries(check_time ${ROOT_LIBRARIES})

# 设置安装路径（可选）
install(TARGETS check_time DESTINATION bin)