#include "render.h"

GLFWwindow *window = nullptr;

static VkInstance                       instance                             = VK_NULL_HANDLE;
static VkPhysicalDevice                 physicalDevice                       = VK_NULL_HANDLE;
static VkPhysicalDeviceProperties       physicalDeviceProperty               = {};
static VkPhysicalDeviceMemoryProperties g_physicalDeviceMemoryProperties     = {};
static VkDevice                         g_device                             = VK_NULL_HANDLE;

static VkQueue                          graphic_queue1                       = VK_NULL_HANDLE;
static VkQueue                          graphic_queue2                       = VK_NULL_HANDLE;
static VkQueue                          graphic_queue3                       = VK_NULL_HANDLE;
static VkQueue                          graphic_queue4                       = VK_NULL_HANDLE;
static VkQueue                          compute_queue                        = VK_NULL_HANDLE;
static VkQueue                          transfer_queue                       = VK_NULL_HANDLE;
static VkQueue                          present_queue                        = VK_NULL_HANDLE;

static VkCommandPool                    commandPool                          = VK_NULL_HANDLE;
static VkCommandBuffer                  commandBuffer                        = VK_NULL_HANDLE;
static std::vector<VkCommandBuffer>     commandBuffers;

static VkSurfaceKHR                     surface                              = VK_NULL_HANDLE;
static VkSurfaceCapabilitiesKHR         surfaceCapabilities                  = {};
static VkSurfaceFormatKHR               surfaceFormat                        = {};

static VkSwapchainKHR                   swapchain                            = VK_NULL_HANDLE;
static std::vector<VkImage>             swapchainImages;
static std::vector<VkImageView>         swapchainImagesViews;
static uint32_t                         activeSwapchainImageId               = 0;

static VkImage                          depthStencilImage                    = VK_NULL_HANDLE;
static VkImageView                      depthStencilImageView                = VK_NULL_HANDLE;
static VkFormat                         depthStensilFormat                   = VK_FORMAT_UNDEFINED;
static bool                             stencilAvailable                     = false;
static VkDeviceMemory                   depthStencilMemory                   = VK_NULL_HANDLE;

static VkRenderPass                     renderPass                           = VK_NULL_HANDLE;
static std::vector<VkFramebuffer>       framebuffers;

static VkFence                          swapchainImageAvailable              = VK_NULL_HANDLE;

static VkPipeline                       graphicPipeline                      = VK_NULL_HANDLE;
static VkPipelineLayout                 graphicPipelineLayout                = VK_NULL_HANDLE;

static VkPipeline                       computePipeline                      = VK_NULL_HANDLE;
static VkPipelineLayout                 computePipelineLayout                = VK_NULL_HANDLE;

static ResourceManager* resourceManager = nullptr;

static VkBuffer vertices_buffer         = VK_NULL_HANDLE;
static VkBuffer index_buffer            = VK_NULL_HANDLE;
static VkBuffer color_buffer            = VK_NULL_HANDLE;

VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

Render* myRender = nullptr;

//Кол-во вложений. Тут 2, так как используется тест глубины
enum {ATTACHMENTS_COUNT = 2};
//Кол-во изображений
static uint32_t g_swapchainImageCount = 3;
//Булева переменная определяет было ли изменение окна
static bool surfaceResized = false;

//Размеры окна
static const int WIDTH = 800;
static const int HEIGHT = 600;

//Настройка и создание окна
void framebufferResizeCallback(GLFWwindow* window, int width, int height){
	surfaceResized = true;
}
void initGLFWWindow(){
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}
void mainLoop(){
	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();


		myRender->draw();




		//beginRender();
		//render();
		//endRender();

	}

	glfwDestroyWindow(window);
	glfwTerminate();
}

//Описатели
void createInstance(){

		using std::vector;
		using std::cout;
		using std::endl;

		if(glfwVulkanSupported() != GLFW_TRUE){
				cout << "Pizda" << endl;
		}

		vector<const char*> requiredInstanceLayers;
		vector<const char*> requiredInstanceExt;

		requiredInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		requiredInstanceExt.push_back("VK_EXT_debug_report");

		uint32_t count = 0;
		const char ** instanceExt_buffer = glfwGetRequiredInstanceExtensions(&count);


		for (uint32_t i = 0; i < count; i++) {
			requiredInstanceExt.push_back(instanceExt_buffer[i]);
		}

		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext            = nullptr;
		applicationInfo.apiVersion       = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pApplicationName = "Test vulkan";
		applicationInfo.apiVersion       = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pEngineName      = "No engine";
		applicationInfo.engineVersion    = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion       = VK_API_VERSION_1_1;

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext                   = nullptr;
		instanceCreateInfo.flags                   = 0;
		instanceCreateInfo.pApplicationInfo        = &applicationInfo;
		instanceCreateInfo.enabledLayerCount       = static_cast<uint32_t>(requiredInstanceLayers.size());
		instanceCreateInfo.ppEnabledLayerNames     = requiredInstanceLayers.data();
		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredInstanceExt.size());
		instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExt.data();

		VkResult vkResult;
		vkResult = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
		if(vkResult != VK_SUCCESS){
				cout << "ERROR::VULKAN::@[createInstance] -> "; errorCheck(vkResult);
		}

}
void destroyInstance(){

		if(instance != VK_NULL_HANDLE){
				vkDestroyInstance(instance, nullptr);
				instance = VK_NULL_HANDLE;
		}

}

//Отладка
PFN_vkCreateDebugReportCallbackEXT  fvkCreateDebugReportCallbackEXT  = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = nullptr;
VkDebugReportCallbackEXT            debug_report                     = nullptr;

VKAPI_ATTR
VkBool32
VKAPI_CALL
vulkanDebugCallback(
		VkDebugReportFlagsEXT      flags,
		VkDebugReportObjectTypeEXT obj_type,
		uint64_t                   src_obj,
		size_t                     location,
		int32_t                    msg_code,
		const char*                layer_prefix,
		const char*                msg,
		void*                      userData    ){


	if(!strncmp(layer_prefix, "Loader Message", 14))
		return VK_FALSE;

	if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		std::cout << "INFO: ";
	if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		std::cout << "WARNING: ";
	if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		std::cout << "PERFORMANCE_WARNING: ";
	if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		std::cout << "ERROR: ";
	if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		std::cout << "DEBUG: ";

	std::cout << "@[" << layer_prefix << "]: ";
	std::cout << msg << std::endl;
	return VK_FALSE;
}
void setupDebug(){
	fvkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

	if(nullptr == fvkCreateDebugReportCallbackEXT){
		std::cout << "ERROR::VULKAN @[setupDebug] -> Cannot fetch debug functions fvkCreateDebugReportCallbackEXT" << std::endl;
	}

	VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
	debugCreateInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugCreateInfo.flags       = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	debugCreateInfo.pfnCallback = vulkanDebugCallback;

	fvkCreateDebugReportCallbackEXT(instance, &debugCreateInfo, nullptr, &debug_report);

}
void deinitDebug(){
	fvkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

	if(nullptr == fvkDestroyDebugReportCallbackEXT){
		std::cout << "ERROR::VULKAN @[setupDebug] -> Cannot fetch debug functions fvkDestroyDebugReportCallbackEXT" << std::endl;
	}

	fvkDestroyDebugReportCallbackEXT(instance, debug_report, nullptr);
	debug_report = nullptr;
}

//Выбор физического устройства
void pickPhysicalDevice(){
	using namespace std;

	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	vector<VkPhysicalDevice> pD(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, pD.data());

	physicalDevice = pD[0];

	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperty);

}

//Создание и уничтожение устройства
void createDevice(){

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &g_physicalDeviceMemoryProperties);

	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
	std::vector<VkQueueFamilyProperties> prop(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, prop.data());


	vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &count, nullptr);
	std::vector<VkExtensionProperties> exProp(count);
	vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &count, exProp.data());

	std::vector<const char*> device_extensions;

	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	device_extensions.push_back("VK_KHR_shader_draw_parameters");

	const float prior[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext            = nullptr;
	queueCreateInfo.flags            = 0;
	queueCreateInfo.queueFamilyIndex = 0;
	queueCreateInfo.queueCount       = 7;
	queueCreateInfo.pQueuePriorities = prior;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext                   = nullptr;
	deviceCreateInfo.flags                   = 0;
	deviceCreateInfo.queueCreateInfoCount    = 1;
	deviceCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
	deviceCreateInfo.enabledLayerCount       = 0;
	deviceCreateInfo.ppEnabledLayerNames     = nullptr;
	deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(device_extensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = device_extensions.data();
	deviceCreateInfo.pEnabledFeatures        = nullptr;

	VkResult vkResult;
	vkResult = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &g_device);
	errorCheck(vkResult);

	vkGetDeviceQueue(g_device, 0, 0, &graphic_queue1);
	vkGetDeviceQueue(g_device, 0, 1, &graphic_queue2);
	vkGetDeviceQueue(g_device, 0, 2, &graphic_queue3);
	vkGetDeviceQueue(g_device, 0, 3, &graphic_queue4);
	vkGetDeviceQueue(g_device, 0, 4, &compute_queue);
	vkGetDeviceQueue(g_device, 0, 5, &transfer_queue);
	vkGetDeviceQueue(g_device, 0, 6, &present_queue);

}
void destroyDevice(){

	if(g_device != VK_NULL_HANDLE){
		vkDestroyDevice(g_device, nullptr);
		g_device = VK_NULL_HANDLE;
	}

}

//Создание и уничтожение поверхности
void createSurface(){
	VkResult vkResult = VK_SUCCESS;
	vkResult = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	errorCheck(vkResult);


	VkBool32 supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &supported);

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());

	surfaceFormat = formats[0];

}
void destroySurface(){
	if(surface != VK_NULL_HANDLE){
		vkDestroySurfaceKHR(instance, surface, nullptr);
		surface = VK_NULL_HANDLE;
	}
}

//Создание и уничтожение swapchain
void createSwapchain(){

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	uint32_t count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
	std::vector<VkPresentModeKHR> present_modes(count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, present_modes.data());
	for(auto& m : present_modes) {
		if(m == VK_PRESENT_MODE_MAILBOX_KHR) presentMode = m;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext                 = nullptr;
	createInfo.flags                 = 0;
	createInfo.surface               = surface;
	createInfo.minImageCount         = g_swapchainImageCount;
	createInfo.imageFormat           = surfaceFormat.format;
	createInfo.imageColorSpace       = surfaceFormat.colorSpace;
	createInfo.imageExtent           = surfaceCapabilities.currentExtent;
	createInfo.imageArrayLayers      = 1;
	createInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices   = nullptr;
	createInfo.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode           = presentMode;
	createInfo.clipped               = VK_TRUE;
	createInfo.oldSwapchain          = swapchain == VK_NULL_HANDLE ? VK_NULL_HANDLE : swapchain;

	VkResult vkResult;
	vkResult = vkCreateSwapchainKHR(g_device, &createInfo, nullptr, &swapchain);
	errorCheck(vkResult);

}
void destroySwapchain(){
	if(swapchain != VK_NULL_HANDLE){
		vkDestroySwapchainKHR(g_device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
}

//Создание и уничтожение изображений
void createSwapchainImages(){

	swapchainImages.resize(g_swapchainImageCount);
	swapchainImagesViews.resize(g_swapchainImageCount);

	VkResult vkResult;
	vkResult = vkGetSwapchainImagesKHR(g_device, swapchain, &g_swapchainImageCount, nullptr);                errorCheck(vkResult);
	vkResult = vkGetSwapchainImagesKHR(g_device, swapchain, &g_swapchainImageCount, swapchainImages.data()); errorCheck(vkResult);

	for (uint32_t i = 0; i < g_swapchainImageCount; i++) {

		VkComponentMapping componentMapping = {};
		componentMapping.r = VK_COMPONENT_SWIZZLE_R;
		componentMapping.g = VK_COMPONENT_SWIZZLE_G;
		componentMapping.b = VK_COMPONENT_SWIZZLE_B;
		componentMapping.a = VK_COMPONENT_SWIZZLE_A;

		VkImageSubresourceRange imageSubresourceRange = {};
		imageSubresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresourceRange.baseMipLevel   = 0;
		imageSubresourceRange.levelCount     = 1;
		imageSubresourceRange.baseArrayLayer = 0;
		imageSubresourceRange.layerCount     = 1;

		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext            = nullptr;
		imageViewCreateInfo.flags            = 0;
		imageViewCreateInfo.image            = swapchainImages.at(i);
		imageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format           = surfaceFormat.format;
		imageViewCreateInfo.components       = componentMapping;
		imageViewCreateInfo.subresourceRange = imageSubresourceRange;

		vkResult = vkCreateImageView(g_device, &imageViewCreateInfo, nullptr, &swapchainImagesViews.at(i)); errorCheck(vkResult);
	}

}
void destroySwapchainImages(){

	for(auto& imageView : swapchainImagesViews){
		if(imageView == VK_NULL_HANDLE) continue;
		vkDestroyImageView(g_device, imageView, nullptr);
	}

}

//Создание и уничтожение теста глубины и трифарета
void createDepthStencilImage(){

	std::vector<VkFormat> try_formats{VK_FORMAT_D32_SFLOAT_S8_UINT,
																		VK_FORMAT_D24_UNORM_S8_UINT,
																		VK_FORMAT_D16_UNORM_S8_UINT,
																		VK_FORMAT_D32_SFLOAT,
																		VK_FORMAT_D16_UNORM
																	 };

	for(auto& f : try_formats){
		VkFormatProperties format_property = {};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, f, &format_property);
		if(format_property.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
			depthStensilFormat = f;
			break;
		}
	}

	if(depthStensilFormat == VK_FORMAT_UNDEFINED){
		std::cout << "ERROR::VULKAN @[createDepthStencilImage] -> depthStensilFormat == VK_FORMAT_UNDEFINED\n";
	}

	if((depthStensilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
		 (depthStensilFormat == VK_FORMAT_D24_UNORM_S8_UINT ) ||
		 (depthStensilFormat == VK_FORMAT_D16_UNORM_S8_UINT ) ||
		 (depthStensilFormat == VK_FORMAT_S8_UINT           )){
		stencilAvailable = true;
	}

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext                 = nullptr;
	imageCreateInfo.flags                 = 0;
	imageCreateInfo.imageType             = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format                = depthStensilFormat;
	imageCreateInfo.extent                = {surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height, 1};
	imageCreateInfo.mipLevels             = 1;
	imageCreateInfo.arrayLayers           = 1;
	imageCreateInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage                 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices   = nullptr;
	imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult vkResult;
	vkResult = vkCreateImage(g_device, &imageCreateInfo, nullptr, &depthStencilImage); errorCheck(vkResult);

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(g_device, depthStencilImage, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext           = nullptr;
	memoryAllocateInfo.allocationSize  = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(g_physicalDeviceMemoryProperties, memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkResult = vkAllocateMemory(g_device, &memoryAllocateInfo, nullptr, &depthStencilMemory); errorCheck(vkResult);
	vkResult = vkBindImageMemory(g_device, depthStencilImage, depthStencilMemory, 0); errorCheck(vkResult);

	VkComponentMapping components = {};
	components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | (stencilAvailable ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
	subresourceRange.baseMipLevel   = 0;
	subresourceRange.levelCount     = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount     = 1;

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext            = nullptr;
	imageViewCreateInfo.flags            = 0;
	imageViewCreateInfo.image            = depthStencilImage;
	imageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format           = depthStensilFormat;
	imageViewCreateInfo.components       = components;
	imageViewCreateInfo.subresourceRange = subresourceRange;

	vkResult = vkCreateImageView(g_device, &imageViewCreateInfo, nullptr, &depthStencilImageView); errorCheck(vkResult);
}
void destroyDepthStencilImage(){

	if(depthStencilImage != VK_NULL_HANDLE){
		vkDestroyImage(g_device, depthStencilImage, nullptr);
		depthStencilImage = VK_NULL_HANDLE;
	}

	if(depthStencilImageView != VK_NULL_HANDLE){
		vkDestroyImageView(g_device, depthStencilImageView, nullptr);
		depthStencilImageView = VK_NULL_HANDLE;
	}

	if(depthStencilMemory != VK_NULL_HANDLE){
		vkFreeMemory(g_device, depthStencilMemory, nullptr);
		depthStencilMemory = VK_NULL_HANDLE;
	}
}

//Создание и уничтожение renderPass
void createRenderPass(){

	VkAttachmentDescription attachments[ATTACHMENTS_COUNT] = {};
	attachments[0].flags          = 0;
	attachments[0].format         = depthStensilFormat;
	attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachments[1].flags          = 0;
	attachments[1].format         = surfaceFormat.format;
	attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//-------------------------------------------------------------------------------

	VkAttachmentReference subPassColorAttachment = {};
	subPassColorAttachment.attachment = 1;
	subPassColorAttachment.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference subPassDepthStencilAttachment = {};
	subPassDepthStencilAttachment.attachment = 0;
	subPassDepthStencilAttachment.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescriptions = {};
	subpassDescriptions.flags                   = 0;
	subpassDescriptions.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptions.inputAttachmentCount    = 0;
	subpassDescriptions.pInputAttachments       = nullptr;
	subpassDescriptions.colorAttachmentCount    = 1;
	subpassDescriptions.pColorAttachments       = &subPassColorAttachment;
	subpassDescriptions.pResolveAttachments     = nullptr;
	subpassDescriptions.pDepthStencilAttachment = &subPassDepthStencilAttachment;
	subpassDescriptions.preserveAttachmentCount = 0;
	subpassDescriptions.pPreserveAttachments    = nullptr;
	//-----------------------------------------------------------------------------------

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext           = nullptr;
	renderPassCreateInfo.flags           = 0;
	renderPassCreateInfo.attachmentCount = ATTACHMENTS_COUNT;
	renderPassCreateInfo.pAttachments    = attachments;
	renderPassCreateInfo.subpassCount    = 1;
	renderPassCreateInfo.pSubpasses      = &subpassDescriptions;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies   = nullptr;

	VkResult vkResult;
	vkResult = vkCreateRenderPass(g_device, &renderPassCreateInfo, nullptr, &renderPass); errorCheck(vkResult);
}
void destroyRenderPass(){

	if(renderPass != VK_NULL_HANDLE){
		vkDestroyRenderPass(g_device, renderPass, nullptr);
		renderPass = VK_NULL_HANDLE;
	}
}

//Создание и уничтожение framebuffer
void createFramebuffer(){

	framebuffers.resize(g_swapchainImageCount);

	for (uint32_t i = 0; i < g_swapchainImageCount; i++) {

		VkImageView attachments[ATTACHMENTS_COUNT] = {};
		attachments[0] = depthStencilImageView;
		attachments[1] = swapchainImagesViews.at(i);

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext           = nullptr;
		framebufferCreateInfo.flags           = 0;
		framebufferCreateInfo.renderPass      = renderPass;
		framebufferCreateInfo.attachmentCount = ATTACHMENTS_COUNT;
		framebufferCreateInfo.pAttachments    = attachments;
		framebufferCreateInfo.width           = surfaceCapabilities.currentExtent.width;
		framebufferCreateInfo.height          = surfaceCapabilities.currentExtent.height;
		framebufferCreateInfo.layers          = 1;
		VkResult vkResult;
		vkResult = vkCreateFramebuffer(g_device, &framebufferCreateInfo, nullptr, &framebuffers.at(i)); errorCheck(vkResult);
	}
}
void destroyFramebuffer(){

	for(auto& framebuffer : framebuffers){
		if(framebuffer != VK_NULL_HANDLE){
			vkDestroyFramebuffer(g_device, framebuffer, nullptr);
			framebuffer = VK_NULL_HANDLE;
		}
	}
}

//Создание и уничтожение объектов синхронизации
void createSynchronizations(){

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;
	fenceCreateInfo.flags = 0;

	vkCreateFence(g_device, &fenceCreateInfo, nullptr, &swapchainImageAvailable);
}
void destroySynchronizations(){

	if(swapchainImageAvailable != VK_NULL_HANDLE){

		vkDestroyFence(g_device, swapchainImageAvailable, nullptr);
		swapchainImageAvailable = VK_NULL_HANDLE;
	}

}

// Создание и уничтожение командного пула
void createCommandPool(){

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext            = nullptr;
	commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = 0;

	VkResult vkResult;
	vkResult = vkCreateCommandPool(g_device, &commandPoolCreateInfo, nullptr, &commandPool); errorCheck(vkResult);

}
void destroyCommandPool(){

	if(commandPool != VK_NULL_HANDLE){
		vkDestroyCommandPool(g_device, commandPool, nullptr);
		commandPool = VK_NULL_HANDLE;
	}

}

// Создание и уничтожение командного буфера
void createCommandBuffer(){

	VkResult vkResult;
	commandBuffers.reserve(g_swapchainImageCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext              = nullptr;
	commandBufferAllocateInfo.commandPool        = commandPool;
	commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = g_swapchainImageCount;

	vkResult = vkAllocateCommandBuffers(g_device, &commandBufferAllocateInfo, commandBuffers.data()); errorCheck(vkResult);

	for(uint32_t i = 0; i < g_swapchainImageCount; i++){
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext            = nullptr;
		commandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		vkResult = vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo); errorCheck(vkResult);

			VkRect2D render_area {};
			render_area.offset.x      = 0;
			render_area.offset.y      = 0;
			render_area.extent.width  = surfaceCapabilities.currentExtent.width;
			render_area.extent.height = surfaceCapabilities.currentExtent.height;

			VkClearValue clearValue[2] = {};
			clearValue[0].depthStencil.depth   = 0.0f;
			clearValue[0].depthStencil.stencil = 0;
			clearValue[1].color.float32[0]     = 0;
			clearValue[1].color.float32[1]     = 0;
			clearValue[1].color.float32[2]     = 0;
			clearValue[1].color.float32[3]     = 0;

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext           = nullptr;
			renderPassBeginInfo.renderPass      = renderPass;
			renderPassBeginInfo.framebuffer     = framebuffers[i];
			renderPassBeginInfo.renderArea      = render_area;
			renderPassBeginInfo.clearValueCount = 2;
			renderPassBeginInfo.pClearValues    = clearValue;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline);
				VkDeviceSize offsets = 0;
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertices_buffer, &offsets);
				vkCmdBindIndexBuffer(commandBuffers[i], index_buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffers[i], 6, 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

		vkResult = vkEndCommandBuffer(commandBuffers[i]); errorCheck(vkResult);
	}
}
void destroyCommandBuffer(){

	for(auto& commandBuffer : commandBuffers){
		if(commandBuffer != VK_NULL_HANDLE){
			vkFreeCommandBuffers(g_device, commandPool, 1, &commandBuffer);
		}
	}

	if(commandBuffer != VK_NULL_HANDLE){
		vkFreeCommandBuffers(g_device, commandPool, 1, &commandBuffer);
		commandBuffer = VK_NULL_HANDLE;
	}
}

//Создание и уничтожение графического конвеера
void createGraphicsPipeline(){

	VkResult vkResult;

	// Шейдерные этапы
	auto shaderCode = readShaderCodeFromFile(R"(/home/sergey/MyProjects/C_Projects/VulkanTest/Shaders/vert.spv)");
	VkShaderModuleCreateInfo vertexShaderModuleCreateInfo = {};
	vertexShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexShaderModuleCreateInfo.codeSize = shaderCode.size();
	vertexShaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule vertexShaderModule;
	vkResult = vkCreateShaderModule(g_device, &vertexShaderModuleCreateInfo, nullptr, &vertexShaderModule); errorCheck(vkResult);

	shaderCode = readShaderCodeFromFile(R"(/home/sergey/MyProjects/C_Projects/VulkanTest/Shaders/frag.spv)");
	VkShaderModuleCreateInfo fragShaderModuleCreateInfo = {};
	fragShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragShaderModuleCreateInfo.codeSize = shaderCode.size();
	fragShaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule fragShaderModule;
	vkResult = vkCreateShaderModule(g_device, &fragShaderModuleCreateInfo, nullptr, &fragShaderModule); errorCheck(vkResult);

	VkPipelineShaderStageCreateInfo vertStageCreateInfo = {};
	vertStageCreateInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageCreateInfo.pNext               = nullptr;
	vertStageCreateInfo.flags               = 0;
	vertStageCreateInfo.stage               = VK_SHADER_STAGE_VERTEX_BIT;
	vertStageCreateInfo.module              = vertexShaderModule;
	vertStageCreateInfo.pName               = "main";
	vertStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo fragStageCreateInfo = {};
	fragStageCreateInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageCreateInfo.pNext               = nullptr;
	fragStageCreateInfo.flags               = 0;
	fragStageCreateInfo.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageCreateInfo.module              = fragShaderModule;
	fragStageCreateInfo.pName               = "main";
	fragStageCreateInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[] = {vertStageCreateInfo, fragStageCreateInfo};
	//----------------------------------------------------------------------------------------------

	// VertexInputState
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding   = 0;
	vertexInputBindingDescription.stride    = sizeof(float) * 2;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
	vertexInputAttributeDescription.location = 0;
	vertexInputAttributeDescription.binding  = 0;
	vertexInputAttributeDescription.format   = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescription.offset   = 0;

	VkPipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pNext                           = nullptr;
	vertexInputState.flags                           = 0;
	vertexInputState.vertexBindingDescriptionCount   = 1;//0;
	vertexInputState.pVertexBindingDescriptions      = &vertexInputBindingDescription;
	vertexInputState.vertexAttributeDescriptionCount = 1;//0;
	vertexInputState.pVertexAttributeDescriptions    = &vertexInputAttributeDescription;//nullptr;


	//----------------------------------------------------------------------------------------------

	// InputAssemblyState
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.pNext                  = nullptr;
	inputAssemblyState.flags                  = 0;
	inputAssemblyState.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;
	//------------------------------------------------------------------------------------------------------

	// ViewPortState
	VkViewport viewport = {};
	viewport.x        = 0;
	viewport.y        = 0;
	viewport.width    = surfaceCapabilities.currentExtent.width;
	viewport.height   = surfaceCapabilities.currentExtent.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = surfaceCapabilities.currentExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext         = nullptr;
	viewportState.flags         = 0;
	viewportState.viewportCount = 1;
	viewportState.pViewports    = &viewport;
	viewportState.scissorCount  = 1;
	viewportState.pScissors     = &scissor;
	//--------------------------------------------------------------------------------------

	// RasterizationState
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.pNext                   = nullptr;
	rasterizationState.flags                   = 0;
	rasterizationState.depthClampEnable        = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode             = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode                = VK_CULL_MODE_NONE;
	rasterizationState.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.depthBiasEnable         = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0f;
	rasterizationState.depthBiasClamp          = 0.0f;
	rasterizationState.depthBiasSlopeFactor    = 0.0f;
	rasterizationState.lineWidth               = 1.0f;
	//------------------------------------------------------------------------------------------------------

	// MultisamplingState
	VkPipelineMultisampleStateCreateInfo multisamplingState = {};
	multisamplingState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingState.pNext                 = nullptr;
	multisamplingState.flags                 = 0;
	multisamplingState.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	multisamplingState.sampleShadingEnable   = VK_FALSE;
	multisamplingState.minSampleShading      = 1.0f;
	multisamplingState.pSampleMask           = nullptr;
	multisamplingState.alphaToCoverageEnable = VK_FALSE;
	multisamplingState.alphaToOneEnable      = VK_FALSE;
	//--------------------------------------------------------------------------------------------------

	// DepthStencilStage
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.pNext                  = nullptr;
	depthStencilState.flags                  = 0;
	depthStencilState.depthTestEnable        = VK_FALSE;
	depthStencilState.depthWriteEnable       = VK_FALSE;
	depthStencilState.depthCompareOp         = VK_COMPARE_OP_NEVER;
	depthStencilState.depthBoundsTestEnable  = VK_FALSE;
	depthStencilState.stencilTestEnable      = VK_FALSE;
	depthStencilState.front                  = {};
	depthStencilState.back                   = {};
	depthStencilState.minDepthBounds         = 1.0f;
	depthStencilState.maxDepthBounds         = 1.0f;
	//-----------------------------------------------------------------------------------------------

	// ColorBlendState
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable         = VK_FALSE;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendState  = {};
	colorBlendState.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.pNext             = nullptr;
	colorBlendState.flags             = 0;
	colorBlendState.logicOpEnable     = VK_FALSE;
	colorBlendState.logicOp           = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount   = 1;
	colorBlendState.pAttachments      = &colorBlendAttachmentState;
	colorBlendState.blendConstants[0] = 0.0f;
	colorBlendState.blendConstants[1] = 0.0f;
	colorBlendState.blendConstants[2] = 0.0f;
	colorBlendState.blendConstants[3] = 0.0f;
	//-----------------------------------------------------------------------------------------------

	// DynamicState
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext             = nullptr;
	dynamicState.flags             = 0;
	dynamicState.dynamicStateCount = 0;
	dynamicState.pDynamicStates    = nullptr;
	//-----------------------------------------------------------------------------------------------

	//Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext                  = nullptr;
	pipelineLayoutCreateInfo.flags                  = 0;
	pipelineLayoutCreateInfo.setLayoutCount         = 0;
	pipelineLayoutCreateInfo.pSetLayouts            = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges    = nullptr;

	vkResult = vkCreatePipelineLayout(g_device, &pipelineLayoutCreateInfo, nullptr, &graphicPipelineLayout); errorCheck(vkResult);

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.pNext               = nullptr;
	graphicsPipelineCreateInfo.flags               = 0;
	graphicsPipelineCreateInfo.stageCount          = 2;
	graphicsPipelineCreateInfo.pStages             = shaderStageCreateInfo;
	graphicsPipelineCreateInfo.pVertexInputState   = &vertexInputState;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	graphicsPipelineCreateInfo.pTessellationState  = nullptr;
	graphicsPipelineCreateInfo.pViewportState      = &viewportState;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
	graphicsPipelineCreateInfo.pMultisampleState   = &multisamplingState;
	graphicsPipelineCreateInfo.pDepthStencilState  = &depthStencilState;
	graphicsPipelineCreateInfo.pColorBlendState    = &colorBlendState;
	graphicsPipelineCreateInfo.pDynamicState       = &dynamicState;
	graphicsPipelineCreateInfo.layout              = graphicPipelineLayout;
	graphicsPipelineCreateInfo.renderPass          = renderPass;
	graphicsPipelineCreateInfo.subpass             = 0;
	graphicsPipelineCreateInfo.basePipelineHandle  = nullptr;
	graphicsPipelineCreateInfo.basePipelineIndex   = -1;

	vkResult = vkCreateGraphicsPipelines(g_device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &graphicPipeline); errorCheck(vkResult);

	vkDestroyShaderModule(g_device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(g_device, fragShaderModule, nullptr);
}
void destroyGraphicsPipeline(){

	if(graphicPipeline != VK_NULL_HANDLE){
		vkDestroyPipeline(g_device, graphicPipeline, nullptr);
		graphicPipeline = VK_NULL_HANDLE;
	}

	if(graphicPipelineLayout != VK_NULL_HANDLE){
		vkDestroyPipelineLayout(g_device, graphicPipelineLayout, nullptr);
		graphicPipelineLayout = VK_NULL_HANDLE;
	}
}

//Создание и уничтожение вычислительного конвеера
void createComputePipeline(){

	VkResult vkResult;
	//Шейдер
	auto shaderCode = readShaderCodeFromFile(R"(/home/sergey/MyProjects/C_Projects/VulkanTest/Shaders/comp.spv)");
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext    = nullptr;
	shaderModuleCreateInfo.flags    = 0;
	shaderModuleCreateInfo.codeSize = shaderCode.size();
	shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule computeShaderModule;
	vkCreateShaderModule(g_device, &shaderModuleCreateInfo, nullptr, &computeShaderModule);

	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.pNext               = nullptr;
	shaderStage.flags               = 0;
	shaderStage.stage               = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStage.module              = computeShaderModule;
	shaderStage.pName               = "main";
	shaderStage.pSpecializationInfo = nullptr;
	//-----------------------------------------------------------------------------------------------

	// Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext                  = nullptr;
	pipelineLayoutCreateInfo.flags                  = 0;
	pipelineLayoutCreateInfo.setLayoutCount         = 0;
	pipelineLayoutCreateInfo.pSetLayouts            = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges    = nullptr;


	vkCreatePipelineLayout(g_device, &pipelineLayoutCreateInfo, nullptr, &computePipelineLayout);
	//-----------------------------------------------------------------------------------------------

	// Pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext              = nullptr;
	computePipelineCreateInfo.flags              = 0;
	computePipelineCreateInfo.stage              = shaderStage;
	computePipelineCreateInfo.layout             = computePipelineLayout;
	computePipelineCreateInfo.basePipelineHandle = nullptr;
	computePipelineCreateInfo.basePipelineIndex  = -1;

	vkResult = vkCreateComputePipelines(g_device, nullptr, 1, &computePipelineCreateInfo, nullptr, &computePipeline); errorCheck(vkResult);
	//-----------------------------------------------------------------------------------------------

	vkDestroyShaderModule(g_device, computeShaderModule, nullptr);
}
void destroyComputePipeline(){
	if(computePipeline != VK_NULL_HANDLE){
		vkDestroyPipeline(g_device, computePipeline, nullptr);
		computePipeline = VK_NULL_HANDLE;
	}
	if(computePipelineLayout != VK_NULL_HANDLE){
		vkDestroyPipelineLayout(g_device, computePipelineLayout, nullptr);
		computePipelineLayout = VK_NULL_HANDLE;
	}
}

//Пересоздание свап чейна после изменения размеров окна
void recreateSwapchain(){

	vkResetFences(g_device, 1, &swapchainImageAvailable);

	int width = 0, height = 0;
	while(width == 0 || height == 0){
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(g_device);
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	//Очистка
	destroyCommandBuffer();
	destroyCommandPool();
	destroyFramebuffer();
	destroyDepthStencilImage();
	destroyGraphicsPipeline();
	destroyRenderPass();
	destroySwapchainImages();
	destroySwapchain();
	//-----------------------

	//Создание
	createSwapchain();
	createSwapchainImages();
	createRenderPass();
	createGraphicsPipeline();
	createDepthStencilImage();
	createFramebuffer();
	createCommandPool();
	createCommandBuffer();
	//-----------------------

}

//Создание и уничтожение менеджера ресурсов
void initResourceManager(){
	resourceManager = new ResourceManager();
	resourceManager->initialization(g_device, physicalDevice, transfer_queue, 0);
}
void DeInitResourceManager(){
	resourceManager->deInitialization();
}

//Создание и уничтожение ресурсов
void createResources(){

	std::vector<float>    vertices  = {-0.5f,-0.5f, 0.5f,-0.5f, 0.5f, 0.5f,-0.5f, 0.5f};
	std::vector<uint32_t> indices   = {0, 1, 2, 0, 2, 3};

	//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	ResourceBufferCreateInfo vertexBufferCreateInfo;
	vertexBufferCreateInfo.bufferUsageFlags   = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferCreateInfo.memoryProperyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	vertexBufferCreateInfo.dataSize           = vertices.size() * sizeof(*vertices.begin());
	vertexBufferCreateInfo.pData              = vertices.data();

	ResourceBufferCreateInfo indexBufferCreateInfo;
	indexBufferCreateInfo.bufferUsageFlags   = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferCreateInfo.memoryProperyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	indexBufferCreateInfo.dataSize           = indices.size() * sizeof(*indices.begin());
	indexBufferCreateInfo.pData              = indices.data();

	vertices_buffer = resourceManager->createBuffer(vertexBufferCreateInfo);
	index_buffer    = resourceManager->createBuffer(indexBufferCreateInfo);
	//-----------------------------------------------------------------------------------------------


}
void destroyResources(){

}

//
void test(){
	myRender = new Render();
	std::vector<float> vertex = {-0.9f, 0.9f,-0.9f, 0.8f,-0.8f, 0.8f};
	ResourceBufferCreateInfo vertexBufferCreateInfo2 = {};
	vertexBufferCreateInfo2.bufferUsageFlags   = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	vertexBufferCreateInfo2.memoryProperyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	vertexBufferCreateInfo2.dataSize           = vertex.size() * sizeof(*vertex.begin());
	vertexBufferCreateInfo2.pData              = vertex.data();

	std::vector<uint32_t> index = {0, 1, 2};
	ResourceBufferCreateInfo indexBufferCreateInfo2;
	indexBufferCreateInfo2.bufferUsageFlags   = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferCreateInfo2.memoryProperyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	indexBufferCreateInfo2.dataSize           = index.size() * sizeof(*index.begin());
	indexBufferCreateInfo2.pData              = index.data();

	myRender->m_pipeline      = graphicPipeline;
	myRender->m_vertex_buffer = resourceManager->createBuffer(vertexBufferCreateInfo2);
	myRender->m_index_buffer  = resourceManager->createBuffer(indexBufferCreateInfo2);
}

//Операции перед и после отрисовки
void preRender(){

	RenderedObject object1;
	RenderedObject object2;
	RenderedObject object3;
	RenderedObject object4;

	//Первый объект
	std::vector<float> vertex1 = {-0.9f, 0.9f,-0.9f, 0.7f, -0.7f, 0.9f};
	VkBuffer vertex_buffer1 = VK_NULL_HANDLE;
	ResourceBufferCreateInfo vertexBufferCreateInfo1 = {};
	vertexBufferCreateInfo1.bufferUsageFlags   = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferCreateInfo1.memoryProperyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	vertexBufferCreateInfo1.dataSize           = vertex1.size() + sizeof(vertex1.at(0));
	vertexBufferCreateInfo1.pData              = vertex1.data();

	vertex_buffer1 = resourceManager->createBuffer(vertexBufferCreateInfo1);

	std::vector<uint32_t> index1 = {0, 1, 2};
	VkBuffer index_buffer1 = VK_NULL_HANDLE;
	ResourceBufferCreateInfo indexBufferCreateInfo1 = {};
	indexBufferCreateInfo1.bufferUsageFlags   = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferCreateInfo1.memoryProperyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	indexBufferCreateInfo1.dataSize           = index1.size() + sizeof(index1.at(0));;
	indexBufferCreateInfo1.pData              = index1.data();

	index_buffer1 = resourceManager->createBuffer(indexBufferCreateInfo1);

	object1.bindBuffers(vertex_buffer1, index_buffer1);
	object1.bindGraphicsPipeline(graphicPipeline);

	RenderedObjectTopology topology1 = {};
	topology1.m_index_count = index1.size();

	object1.setTopology(topology1);
	object1.initObject();
	//-----------------------------------------------------------------------------------------------


	myRender = new Render();
	myRender->add(object1);
	myRender->initialize();
	//myRender->add(object2);
	//myRender->add(object3);
	//myRender->add(object4);

}
void postRender(){
	vkDeviceWaitIdle(g_device);
}

//Начало и конец отрисовки
void beginRender(){

	VkResult vkResult;
	vkResult = vkAcquireNextImageKHR(g_device, swapchain, ~0ull, VK_NULL_HANDLE, swapchainImageAvailable, &activeSwapchainImageId); errorCheck(vkResult);

	if(vkResult == VK_ERROR_OUT_OF_DATE_KHR){
		surfaceResized = false;
		recreateSwapchain();
	}

	vkResult = vkWaitForFences(g_device, 1, &swapchainImageAvailable, VK_TRUE, ~0ull); errorCheck(vkResult);
	vkResult = vkResetFences(g_device, 1, &swapchainImageAvailable); errorCheck(vkResult);

}
void render(){


}
void endRender(){

	VkSubmitInfo submitInfo = {};
	submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext                = nullptr;
	submitInfo.waitSemaphoreCount   = 0;
	submitInfo.pWaitSemaphores      = nullptr;
	submitInfo.pWaitDstStageMask    = nullptr;
	submitInfo.commandBufferCount   = 1;
	submitInfo.pCommandBuffers      = &commandBuffers[activeSwapchainImageId];
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores    = nullptr;

	vkQueueSubmit(graphic_queue1, 1, &submitInfo, swapchainImageAvailable);
	vkWaitForFences(g_device, 1, &swapchainImageAvailable, VK_TRUE, ~static_cast<uint64_t>(0));
	vkResetFences(g_device, 1, &swapchainImageAvailable);

	VkPresentInfoKHR present_info {};
	present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext              = nullptr;
	present_info.waitSemaphoreCount = 0;
	present_info.pWaitSemaphores    = VK_NULL_HANDLE;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = &swapchain;
	present_info.pImageIndices      = &activeSwapchainImageId;
	present_info.pResults           = nullptr;

	VkResult vkResult;
	vkResult = vkQueuePresentKHR(present_queue, &present_info);

	if(vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR){
		recreateSwapchain();
	}
}

//Инициализация и деинициализация Vulkan
void vk_init(){

	createInstance();
	setupDebug();
	pickPhysicalDevice();
	createDevice();

	initResourceManager();
	createResources();

	createSurface();
	createSwapchain();
	createSwapchainImages();
	createDepthStencilImage();
	createRenderPass();
	createFramebuffer();
	createSynchronizations();
	createGraphicsPipeline();
	createComputePipeline();
	createCommandPool();
	createCommandBuffer();

	test();

}
void vk_deinit(){

	destroyComputePipeline();
	destroyGraphicsPipeline();
	destroySynchronizations();
	destroyFramebuffer();
	destroyRenderPass();
	destroyCommandBuffer();
	destroyCommandPool();
	destroyDepthStencilImage();
	destroySwapchain();
	destroySwapchainImages();
	destroySurface();
	deinitDebug();

	DeInitResourceManager();

	destroyDevice();
	destroyInstance();
}


//Rendered objects
void RenderedObject :: initObject(){

	VkCommandBufferAllocateInfo allocate_info = {};
	allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.pNext              = nullptr;
	allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocate_info.commandPool        = commandPool;
	allocate_info.commandBufferCount = 1;

	vkAllocateCommandBuffers(g_device, &allocate_info, &m_commandBuffer);

	VkCommandBufferInheritanceInfo inheritanceInfo = {};;
	inheritanceInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.pNext                = nullptr;
	inheritanceInfo.renderPass           = renderPass;
	inheritanceInfo.subpass              = VK_NULL_HANDLE;
	inheritanceInfo.framebuffer          = VK_NULL_HANDLE;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.queryFlags           = 0;
	inheritanceInfo.pipelineStatistics   = 0;

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext            = nullptr;
	begin_info.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	begin_info.pInheritanceInfo = &inheritanceInfo;

	vkBeginCommandBuffer(m_commandBuffer, &begin_info);

			VkDeviceSize offsets = 0;
			vkCmdBindPipeline     (m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
			vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &m_vertexBuffer, &offsets);
			vkCmdBindIndexBuffer  (m_commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed      (m_commandBuffer, m_topology.m_index_count, 1, 0, 0, 0);

	vkEndCommandBuffer(m_commandBuffer);

}

void RenderedObject::bindGraphicsPipeline(const VkPipeline _graphics_pipeline){
	m_pipeline = _graphics_pipeline;
}

void RenderedObject::bindVertexBuffer(VkBuffer _vertexBuffer){
	m_vertexBuffer = _vertexBuffer;
}

void RenderedObject::bindIndexBuffer(VkBuffer _indexBuffer){
	m_indexBuffer = _indexBuffer;
}

void RenderedObject::bindBuffers(VkBuffer _vertexBuffer, VkBuffer _indexBuffer){
	bindVertexBuffer(_vertexBuffer);
	bindIndexBuffer(_indexBuffer);
}

VkCommandBuffer RenderedObject::getCommandBuffer(){
	return m_commandBuffer;
}


//Render
void Render::initialize(){

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext            = nullptr;
	commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = 0;

	vkCreateCommandPool(g_device, &commandPoolCreateInfo, nullptr, &m_main_command_pool);

	//Вторичные командные буфера
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext              = nullptr;
	alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	alloc_info.commandPool        = m_main_command_pool;
	alloc_info.commandBufferCount = 1;

	vkAllocateCommandBuffers(g_device, &alloc_info, &m_secondary_command_buffer);

	VkCommandBufferInheritanceInfo inheritanceInfo;
	inheritanceInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.pNext                = nullptr;
	inheritanceInfo.renderPass           = renderPass;
	inheritanceInfo.subpass              = VK_NULL_HANDLE;
	inheritanceInfo.framebuffer          = VK_NULL_HANDLE;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.queryFlags           = 0;
	inheritanceInfo.pipelineStatistics   = 0;

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext            = nullptr;
	begin_info.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	begin_info.pInheritanceInfo = &inheritanceInfo;

	vkBeginCommandBuffer(m_secondary_command_buffer, &begin_info);

		vkCmdBindPipeline(m_secondary_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline);
		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(m_secondary_command_buffer, 0, 1, &vertices_buffer, &offsets);
		vkCmdBindIndexBuffer(m_secondary_command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(m_secondary_command_buffer, 6, 1, 0, 0, 0);

	vkEndCommandBuffer(m_secondary_command_buffer);
	//--------------------------

	m_main_command_buffers.resize(g_swapchainImageCount);
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = m_main_command_pool;
	commandBufferAllocateInfo.commandBufferCount = g_swapchainImageCount;

	vkAllocateCommandBuffers(g_device, &commandBufferAllocateInfo, m_main_command_buffers.data());

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext            = nullptr;
	commandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = 0;

	size_t i = 0;
	for(auto& command_buffer : m_main_command_buffers){

		vkBeginCommandBuffer(command_buffer, &commandBufferBeginInfo);

			VkRect2D render_area {};
			render_area.offset.x      = 0;
			render_area.offset.y      = 0;
			render_area.extent.width  = surfaceCapabilities.currentExtent.width;
			render_area.extent.height = surfaceCapabilities.currentExtent.height;

			VkClearValue clearValue[2] = {};
			clearValue[0].depthStencil.depth   = 0.0f;
			clearValue[0].depthStencil.stencil = 0;
			clearValue[1].color.float32[0]     = 0;
			clearValue[1].color.float32[1]     = 0;
			clearValue[1].color.float32[2]     = 0;
			clearValue[1].color.float32[3]     = 0;

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext           = nullptr;
			renderPassBeginInfo.renderPass      = renderPass;
			renderPassBeginInfo.framebuffer     = framebuffers[i];
			renderPassBeginInfo.renderArea      = render_area;
			renderPassBeginInfo.clearValueCount = 2;
			renderPassBeginInfo.pClearValues    = clearValue;

			vkCmdBeginRenderPass(command_buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

				vkCmdExecuteCommands(command_buffer, 1, &m_secondary_command_buffer);

			vkCmdEndRenderPass(command_buffer);

		vkEndCommandBuffer(command_buffer);

		i++;
	}
}

void Render::draw(){

	vkResult = vkAcquireNextImageKHR(g_device, swapchain, ~0ull, VK_NULL_HANDLE, swapchainImageAvailable, &activeSwapchainImageId); errorCheck(vkResult);

	if(vkResult == VK_ERROR_OUT_OF_DATE_KHR){
		surfaceResized = false;
		recreateSwapchain();
	}

	vkResult = vkWaitForFences(g_device, 1, &swapchainImageAvailable, VK_TRUE, ~0ull); errorCheck(vkResult);
	vkResult = vkResetFences(g_device, 1, &swapchainImageAvailable); errorCheck(vkResult);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext                = nullptr;
	submitInfo.waitSemaphoreCount   = 0;
	submitInfo.pWaitSemaphores      = nullptr;
	submitInfo.pWaitDstStageMask    = nullptr;
	submitInfo.commandBufferCount   = 1;
	submitInfo.pCommandBuffers      = &m_main_command_buffers[activeSwapchainImageId];
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores    = nullptr;

	vkQueueSubmit  (graphic_queue1, 1, &submitInfo, swapchainImageAvailable);
	vkWaitForFences(g_device, 1, &swapchainImageAvailable, VK_TRUE, ~static_cast<uint64_t>(0));
	vkResetFences  (g_device, 1, &swapchainImageAvailable);

	VkPresentInfoKHR present_info {};
	present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext              = nullptr;
	present_info.waitSemaphoreCount = 0;
	present_info.pWaitSemaphores    = VK_NULL_HANDLE;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = &swapchain;
	present_info.pImageIndices      = &activeSwapchainImageId;
	present_info.pResults           = nullptr;

	vkResult = vkQueuePresentKHR(present_queue, &present_info);

	if(vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR){
		recreateSwapchain();
	}



}

void Render::add(const RenderedObject _renderedObject){
	m_rendered_objects.push_back(_renderedObject);
}






















