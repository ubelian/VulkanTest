#include "mybuffer.hpp"



memoryProperties ResourceManager::bindBufferMemory(VkBuffer &_buffer,
																									 VkDeviceSize _bufferSize,
																									 VkBufferUsageFlags _usage,
																									 uint32_t _property){

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(m_device, _buffer, &memoryRequirements);

	uint32_t index = FindMemoryTypeIndex(m_physicalDeviceMemoryProperties, memoryRequirements, _property);
	MemoryEntity& me = m_memory_MemoryEntity.at(index);
	if(me.deviceMemory == VK_NULL_HANDLE){

		//Выделение памяти
		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext           = nullptr;
		memoryAllocateInfo.allocationSize  = DEFAULT_ALLOCATED_MEMORY_SIZE;
		memoryAllocateInfo.memoryTypeIndex = index;
		vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &me.deviceMemory);
		//-------------------------------------------------------------------------------------------

		//Установка сегментации
		me.segmentations.push_back({0, DEFAULT_ALLOCATED_MEMORY_SIZE - 1});
		//-------------------------------------------------------------------------------------------
	}
	memoryProperties bam;
	//Поиск подходящего сегмента в памяти, в который поместится новый буфер
	auto rbegin = me.segmentations.rbegin();
	auto rend   = me.segmentations.rend();
	auto factualSize = memoryRequirements.size;
	for(auto i = rbegin; i != rend; i++){
		if((i->second - i->first) >= factualSize){
			vkBindBufferMemory(m_device, _buffer, me.deviceMemory, i->first);
			bam.m_offset = i->first;
			i->first = i->first + factualSize;
			bam.m_realSize = _bufferSize;
			ResourceBuffer resourceBuffer;
			resourceBuffer.buffer      = _buffer;
			resourceBuffer.realSize    = _bufferSize;
			resourceBuffer.factualSize = factualSize;
			me.list_of_resouces_buffers.push_back(resourceBuffer);
			break;
		}
	}

	bam.m_memory = &me.deviceMemory;
	return bam;
}

void ResourceManager::initialization(VkDevice &_device,
																		 VkPhysicalDevice &_physicalDevice,
																		 VkQueue &_transferQueue,
																		 uint32_t _transferQueueFamilyIndex){

	m_device         = _device;
	m_physicalDevice = _physicalDevice;
	m_transferQueue  = _transferQueue;

	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.pNext            = nullptr;
	createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = _transferQueueFamilyIndex;
	vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool);

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext              = nullptr;
	allocateInfo.commandPool        = m_commandPool;
	allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(m_device, &allocateInfo, &m_commandBuffer);

	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);
	m_memory_MemoryEntity.resize(m_physicalDeviceMemoryProperties.memoryTypeCount);

	//Инициализация staging_buffer и staging_buffer_memory

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext                 = nullptr;
	bufferCreateInfo.flags                 = 0;
	bufferCreateInfo.size                  = DEFAULT_ALLOCATED_STAGING_MEMORY_AND_BUFFER_SIZE;
	bufferCreateInfo.usage                 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices   = nullptr;
	vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_staging_buffer);

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(m_device, m_staging_buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext           = nullptr;
	memoryAllocateInfo.allocationSize  = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(m_physicalDeviceMemoryProperties, memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_staging_buffer_memory);

	vkBindBufferMemory(m_device, m_staging_buffer, m_staging_buffer_memory, 0);

}

void ResourceManager::deInitialization(){

	if(m_staging_buffer != VK_NULL_HANDLE){
		vkDestroyBuffer(m_device, m_staging_buffer, nullptr);
		m_staging_buffer = VK_NULL_HANDLE;
	}

	if(m_staging_buffer_memory != VK_NULL_HANDLE){
		vkFreeMemory(m_device, m_staging_buffer_memory, nullptr);
		m_staging_buffer_memory = VK_NULL_HANDLE;
	}

	if(m_commandBuffer != VK_NULL_HANDLE){
		vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
		m_commandBuffer = VK_NULL_HANDLE;
	}

	if(m_commandPool != VK_NULL_HANDLE){
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		m_commandPool = VK_NULL_HANDLE;
	}

	for (auto& me : m_memory_MemoryEntity) {
		if(me.deviceMemory != VK_NULL_HANDLE){
			for (auto& ResourceBuffer : me.list_of_resouces_buffers) {
				if(ResourceBuffer.buffer != VK_NULL_HANDLE){
					vkDestroyBuffer(m_device, ResourceBuffer.buffer, nullptr);
					ResourceBuffer.buffer = VK_NULL_HANDLE;
				}
			}
			vkFreeMemory(m_device, me.deviceMemory, nullptr);
			me.deviceMemory = VK_NULL_HANDLE;
		}
	}
}

VkBuffer ResourceManager::createBuffer(ResourceBufferCreateInfo &_bufferCreateInfo){

	if(_bufferCreateInfo.dataSize == 0){
		std::cout << "VULKAN::WARNING @[createBuffer] -> Input array is empty" << std::endl;
	}

	//Если буфер находится в device_local_bit, то добавить флаг к _usage VK_BUFFER_USAGE
	if(_bufferCreateInfo.memoryProperyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){
		_bufferCreateInfo.bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext                 = nullptr;
	bufferCreateInfo.flags                 = 0;
	bufferCreateInfo.size                  = _bufferCreateInfo.dataSize;
	bufferCreateInfo.usage                 = _bufferCreateInfo.bufferUsageFlags;
	bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices   = nullptr;

	VkBuffer buffer;
	VkResult vkResult = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &buffer); errorCheck(vkResult);

	auto bam = bindBufferMemory(buffer, bufferCreateInfo.size, _bufferCreateInfo.bufferUsageFlags, _bufferCreateInfo.memoryProperyFlags);

	//Если буфер должен находится в памяти, видимой цпу, то
	if(_bufferCreateInfo.memoryProperyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)){

		void* data;
		vkMapMemory(m_device, *bam.m_memory, bam.m_offset, bufferCreateInfo.size, 0, &data);
		memcpy(data, _bufferCreateInfo.pData, static_cast<size_t>(bufferCreateInfo.size));
		vkUnmapMemory(m_device, *bam.m_memory);

		return buffer;

	}

	//Если буфер должен находится в памяти видеокарты, то
	if(_bufferCreateInfo.memoryProperyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){

		void* data;
		vkMapMemory(m_device, m_staging_buffer_memory, 0, bufferCreateInfo.size, 0, &data);
		memcpy(data, _bufferCreateInfo.pData, static_cast<size_t>(bufferCreateInfo.size));
		vkUnmapMemory(m_device, m_staging_buffer_memory);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext            = nullptr;
		beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size      = bam.m_realSize;
		vkCmdCopyBuffer(m_commandBuffer, m_staging_buffer, buffer, 1, &copyRegion); //Bcghfdbnmn

		vkEndCommandBuffer(m_commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &m_commandBuffer;
		vkQueueSubmit(m_transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_transferQueue);
		vkResetCommandBuffer(m_commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		//vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);

		return buffer;
	}

	return buffer;
}
