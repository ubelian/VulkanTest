#ifndef RENDER_H
#define RENDER_H

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include"utilies.hpp"
#include"mybuffer.hpp"

#include<vector>
#include<list>
#include<iostream>
#include<string.h>

#include<glm/matrix.hpp>
#include<glm/gtc/matrix_transform.hpp>

void framebufferResizeCallback(GLFWwindow* window, int width, int height);
void initGLFWWindow();
void mainLoop();

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
		void*                      userData    );
void setupDebug();
void deinitDebug();

//Выбор физического устройства
void pickPhysicalDevice();

//Создание и уничтожение устройства
void createDevice();
void destroyDevice();

//Создание и уничтожение поверхности
void createSurface();
void destroySurface();

//Создание и уничтожение swapchain
void createSwapchain();
void destroySwapchain();

//Создание и уничтожение изображений
void createSwapchainImages();
void destroySwapchainImages();

//Создание и уничтожение теста глубины и трифарета
void createDepthStencilImage();

void destroyDepthStencilImage();

//Создание и уничтожение renderPass
void createRenderPass();
void destroyRenderPass();

//Создание и уничтожение framebuffer
void createFramebuffer();
void destroyFramebuffer();

//Создание и уничтожение объектов синхронизации
void createSynchronizations();
void destroySynchronizations();

// Создание и уничтожение командного пула
void createCommandPool();
void destroyCommandPool();

// Создание и уничтожение командного буфера
void createCommandBuffer();
void destroyCommandBuffer();

//Создание и уничтожение графического конвеера
void createGraphicsPipeline();
void destroyGraphicsPipeline();

//Создание и уничтожение вычислительного конвеера
void createComputePipeline();
//Пересоздание свап чейна после изменения размеров окна
void recreateSwapchain();

//Создание и уничтожение менеджера ресурсов
void initResourceManager();
void DeInitResourceManager();

//Создание и уничтожение ресурсов
void createResources();
void destroyResources();

//Операции перед и после отрисовки
void preRender();
void postRender();

//Начало и конец отрисовки
void beginRender();
void render();
void endRender();

//Инициализация и деинициализация Vulkan
void vk_init();
void vk_deinit();



struct RenderedObjectTopology{
	size_t              m_topology     = 0;
	uint32_t            m_vertex_count = 0;
	uint32_t            m_index_count  = 0;
};

class RenderedObject{
private:
	//Топология объекта определяющая что, как и сколько нужно для отрисовки объекта
	RenderedObjectTopology m_topology      = {};
	//Вершины и индексы
	VkBuffer               m_vertexBuffer  = VK_NULL_HANDLE;
	VkBuffer               m_indexBuffer   = VK_NULL_HANDLE;

	//Конвеер который используется для отрисовки
	VkPipeline             m_pipeline      = VK_NULL_HANDLE;

	//Изображение которое будет использоваться в кач-ве текстуры
	VkImage                m_image         = VK_NULL_HANDLE;

	//Коммандный буфер который будет исользоваться для отрисовки
	//конкретного объекта
	VkCommandBuffer        m_commandBuffer = VK_NULL_HANDLE;
public:

	void initObject();
	void deinitObject();

	void bindGraphicsPipeline(const VkPipeline _graphics_pipeline);
	void bindVertexBuffer    (const VkBuffer   _vertexBuffer);
	void bindIndexBuffer     (const VkBuffer   _indexBuffer);
	void bindBuffers         (const VkBuffer   _vertexBuffer, const VkBuffer _indexBuffer);

	void setTopology(const RenderedObjectTopology& _topology){
		m_topology = _topology;
	}

	VkCommandBuffer getCommandBuffer();

};

class Render{
public:
//private:
	enum{COUNT_OF_COMMAND_BUFFER = 3};
	std::list<RenderedObject>    m_rendered_objects;

	VkCommandPool                m_main_command_pool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_main_command_buffers;

	VkResult vkResult = VK_SUCCESS;


	VkBuffer        m_vertex_buffer            = VK_NULL_HANDLE;
	VkBuffer        m_index_buffer             = VK_NULL_HANDLE;
	VkPipeline      m_pipeline                 = VK_NULL_HANDLE;
	VkCommandBuffer m_secondary_command_buffer = VK_NULL_HANDLE;


public:
	//Инициализация
	void initialize();

	//Команда для отрисовки.
	void draw();

	//Добавление объекта для отрисовки
	void add(const RenderedObject _renderedObject);


};










































#endif // RENDER_H
