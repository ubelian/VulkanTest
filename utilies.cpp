#include"utilies.hpp"

//Проверка на ошибки
void errorCheck(const VkResult& vkResult){
		using std::cout;
		using std::endl;

	if(vkResult == VK_SUCCESS)
		return;

	switch(vkResult){
			case VK_ERROR_OUT_OF_HOST_MEMORY:
					cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << endl;
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << endl;
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
					cout << "VK_ERROR_INITIALIZATION_FAILED" << endl;
			break;
			case VK_ERROR_DEVICE_LOST:
					cout << "VK_ERROR_DEVICE_LOST" << endl;
			break;
			case VK_ERROR_MEMORY_MAP_FAILED:
					cout << "VK_ERROR_MEMORY_MAP_FAILED" << endl;
			break;
			case VK_ERROR_LAYER_NOT_PRESENT:
					cout << "VK_ERROR_LAYER_NOT_PRESENT" << endl;
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
					cout << "VK_ERROR_EXTENSION_NOT_PRESENT" << endl;
			break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
					cout << "VK_ERROR_FEATURE_NOT_PRESENT" << endl;
			break;
			case VK_ERROR_INCOMPATIBLE_DRIVER:
					cout << "VK_ERROR_INCOMPATIBLE_DRIVER" << endl;
			break;
			case VK_ERROR_TOO_MANY_OBJECTS:
					cout << "VK_ERROR_TOO_MANY_OBJECTS" << endl;
			break;
			case VK_ERROR_FORMAT_NOT_SUPPORTED:
					cout << "VK_ERROR_FORMAT_NOT_SUPPORTED" << endl;
			break;
			case VK_ERROR_FRAGMENTED_POOL:
					cout << "VK_ERROR_FRAGMENTED_POOL" << endl;
			break;
			case VK_ERROR_OUT_OF_POOL_MEMORY:
					cout << "VK_ERROR_OUT_OF_POOL_MEMORY" << endl;
			break;
			case VK_ERROR_INVALID_EXTERNAL_HANDLE:
					cout << "VK_ERROR_INVALID_EXTERNAL_HANDLE" << endl;
			break;
			case VK_ERROR_SURFACE_LOST_KHR:
					cout << "VK_ERROR_SURFACE_LOST_KHR" << endl;
			break;
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
					cout << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" << endl;
			break;
			case VK_ERROR_OUT_OF_DATE_KHR:
					cout << "VK_ERROR_OUT_OF_DATE_KHR" << endl;
			break;
			case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
					cout << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" << endl;
			break;
			case VK_ERROR_VALIDATION_FAILED_EXT:
					cout << "VK_ERROR_VALIDATION_FAILED_EXT" << endl;
			break;
			case VK_ERROR_INVALID_SHADER_NV:
					cout << "VK_ERROR_INVALID_SHADER_NV" << endl;
			break;
			case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
					cout << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT" << endl;
			break;
			case VK_ERROR_FRAGMENTATION_EXT:
					cout << "VK_ERROR_FRAGMENTATION_EXT" << endl;
			break;
			case VK_ERROR_NOT_PERMITTED_EXT:
					cout << "VK_ERROR_NOT_PERMITTED_EXT" << endl;
			break;
			case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
					cout << "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT" << endl;
			break;
			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
					cout << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" << endl;
			break;
			default:
			break;
	}
}

//Поиск памяти для выделения
uint32_t FindMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
														 const VkMemoryRequirements&             memoryRequirements,
														 const VkMemoryPropertyFlags&            requiredMemoryProperties){

	for(uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++){
		if(memoryRequirements.memoryTypeBits & ( 1 << i) ){
			if((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & requiredMemoryProperties) == requiredMemoryProperties ){
				return i;
			}
		}
	}
	std::cout << "ERROR::VULKAN @[] -> cannot find a memoryIndex\n";
	return UINT32_MAX;
}

//Чтение шейдера из файла
std::vector<char> readShaderCodeFromFile(const std::string& filename){

	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if(!file.is_open()){
		std::cout << "ERROR::VULKAN:: @[readShaderCodeFromFile] -> Cannot to open file: " << filename << std::endl;
		return {};
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}


