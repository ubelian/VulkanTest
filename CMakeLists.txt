cmake_minimum_required(VERSION 2.8)

project(VulkanTest)

set(VULKAN_SDK_PATH /home/sergey/library/VulkanApi/1.1.106.0/x86_64)
set(BOOST_PATH /home/sergey/library/boost)
set(GLM_PATH /home/sergey/library/GLM/glm-0.9.9.5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${VULKAN_SDK_PATH}/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${BOOST_PATH}/lib")

set(SOURCES main.cpp mybuffer.cpp utilies.cpp render.cpp)
set(HEADERS main.hpp utilies.hpp mybuffer.hpp render.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${VULKAN_SDK_PATH}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${BOOST_PATH}/include)
target_include_directories(${PROJECT_NAME} PUBLIC GLM_PATH)

target_link_libraries(${PROJECT_NAME} glfw vulkan)


