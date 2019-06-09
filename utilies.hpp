#ifndef UTILIES_H
#define UTILIES_H
#include<vulkan/vulkan.h>
#include<GLFW/glfw3.h>

#include<iostream>
#include<fstream>
#include<vector>

//Проверка на ошибки
void errorCheck(const VkResult& vkResult);

//Поиск памяти для выделения
uint32_t FindMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
														 const VkMemoryRequirements&             memoryRequirements,
														 const VkMemoryPropertyFlags&            requiredMemoryProperties);

//Чтение шейдера из файла
std::vector<char> readShaderCodeFromFile(const std::string& filename);


















#endif // UTILIES_H

