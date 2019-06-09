#ifndef MYBUFFER_HPP
#define MYBUFFER_HPP
#include <vulkan/vulkan.h>
#include <vector>
#include <memory.h>

#include "utilies.hpp"

enum{DEFAULT_ALLOCATED_MEMORY_SIZE = 1024 * 1024 * 10, DEFAULT_ALLOCATED_STAGING_MEMORY_AND_BUFFER_SIZE = 1024 * 1024 * 10};

/***************************************************
	Структура необходимая для создания буфера.
	Если pData == nullptr, то в буфер данные записаны
	не будут
***************************************************/
struct ResourceBufferCreateInfo{
	//Использование буфера
	VkBufferUsageFlags       bufferUsageFlags   = VK_NULL_HANDLE;
	//Место в памяти
	uint32_t                 memoryProperyFlags = 0;
	//Размер данных
	size_t                   dataSize           = 0;
	//Указатель на данные, которые будут скопированы
	void*                    pData              = nullptr;
};
//--------------------------------------------------

struct memoryProperties{
	VkDeviceMemory* m_memory   = nullptr;
	VkDeviceSize    m_offset   = 0;
	VkDeviceSize    m_realSize = 0;
};

struct ResourceBuffer{
	//Описатель буфера
	VkBuffer         buffer          = VK_NULL_HANDLE;
	//Реальный размер данных в буфере
	VkDeviceSize     realSize        = 0;
	//Фактический размер буфера в памяти с учетом выравнивания
	VkDeviceSize     factualSize     = 0;
};

struct MemoryEntity{
	VkDeviceMemory                                     deviceMemory = VK_NULL_HANDLE;
	std::vector<ResourceBuffer>                        list_of_resouces_buffers;
	std::vector<std::pair<VkDeviceSize ,VkDeviceSize>> segmentations;
};

class ResourceManager{
private:
	//Объекты необходимые для работы
	VkDevice                         m_device                = VK_NULL_HANDLE;
	VkQueue                          m_transferQueue         = VK_NULL_HANDLE;
	VkPhysicalDevice                 m_physicalDevice        = VK_NULL_HANDLE;

	//Пул и буфер для отправки данных на device_local_bit
	VkCommandPool                    m_commandPool           = VK_NULL_HANDLE;
	VkCommandBuffer                  m_commandBuffer         = VK_NULL_HANDLE;
	//
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties = {};
	//
	std::vector<MemoryEntity>        m_memory_MemoryEntity;

	//
	VkBuffer                         m_staging_buffer        = VK_NULL_HANDLE;
	VkDeviceMemory                   m_staging_buffer_memory = VK_NULL_HANDLE;

	memoryProperties bindBufferMemory(VkBuffer& _buffer, VkDeviceSize _bufferSize, VkBufferUsageFlags _usage, uint32_t _property);

public:

	ResourceManager(){}
 ~ResourceManager(){}

	//Инициализация и деинициализация
	void initialization(VkDevice& _device,
											VkPhysicalDevice& _physicalDevice,
											VkQueue& _transferQueue,
											uint32_t _transferQueueFamilyIndex);
	void deInitialization();

	VkBuffer createBuffer(ResourceBufferCreateInfo& _bufferCreateInfo);

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

};


#endif // MYBUFFER_HPP
