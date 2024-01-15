// pruebaVulkan.cpp: define el punto de entrada de la aplicación de consola.

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <cstring>
#include <fstream>
#include <string>

struct Vertex {
	glm::vec2 mPos;
	glm::vec3 mColor;
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof (Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {

		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, mPos);

		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, mColor);

		return attributeDescriptions;
	}
};

const int FRAMES_IN_FLIGHT = 2;

const std::vector<Vertex> mTriangleVertices =
{
	{{-0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> mIndices =
{
	0, 1, 2, 2, 3, 0
};

bool mNeedToRecreateSwapchain = false;
unsigned int mGPUSelected = 0;
unsigned int mGraphicsQueueFamilyIndex = 0;
unsigned int mTransferQueueFamilyIndex = 0;
unsigned int mCurrentLocalFrame = 0;
unsigned int mSwapchainImagesCount;
const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };
std::vector<VkDynamicState> mDynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
std::vector<const char*> mDeviceExtensions;
std::vector<const char*> mInstanceExtensions;
std::vector< VkDeviceQueueCreateInfo> mQueueCreateInfos;
std::vector<VkImage> mSwapChainImages;
std::vector<VkImageView> mSwapChainImagesViews;
std::vector<VkFramebuffer> mSwapChainFramebuffers;
std::vector<VkPhysicalDevice> mPhysicalDevices;
auto mBindingDescription = Vertex::getBindingDescription();
auto mAttributeDescriptions = Vertex::getAttributeDescriptions();
GLFWwindow* mWindow;
VkResult mPresentResult;
VkInstance mInstance;
VkDebugUtilsMessengerEXT mDebugMessenger {};
VkDevice mLogicDevice;
VkSurfaceKHR mSurface;
VkSurfaceFormatKHR mSurfaceFormat;
VkExtent2D mCurrentExtent;
VkViewport mViewport {};
VkRect2D mScissor {};
VkPresentModeKHR mPresentMode = VK_PRESENT_MODE_FIFO_KHR;
VkSurfaceCapabilitiesKHR mCapabilities;
VkSwapchainCreateInfoKHR mSwapChainCreateInfo{};
VkSwapchainKHR mSwapChain;
VkQueue mGraphicsQueue;
VkQueue mTransferQueue;
VkPipelineLayout mPipelineLayout;
VkRenderPass mRenderPass;
VkPipeline mGraphicsPipeline;
VkCommandPool mCommandPool;
VkQueue mPresentQueue;
VkBuffer mVertexBuffer;
VkBuffer mStagingBuffer;
VkBuffer mIndexBuffer;
VkDeviceMemory mVertexBufferMemory;
VkDeviceMemory mStaggingBufferMemory;
VkDeviceMemory mIndexBufferMemory;
VkPhysicalDeviceMemoryProperties mMemProps;

// Para tener mas de un Frame, cada frame debe tener su pack de semaforos y Fencesnot
VkCommandBuffer mCommandBuffer[FRAMES_IN_FLIGHT];
VkSemaphore mImageAvailable[FRAMES_IN_FLIGHT];
VkSemaphore mRenderFinish[FRAMES_IN_FLIGHT];
VkFence		mInFlight[FRAMES_IN_FLIGHT];

std::vector<char> LoadShader(const std::string& _filename)
{
	std::ifstream f(_filename, std::ios::ate | std::ios::binary);
	size_t fileSize = (size_t)f.tellg();
	std::vector<char> buffer(fileSize);
	f.seekg(0);
	f.read(buffer.data(), fileSize);
	f.close();
	return buffer;
}

bool checkValidationLayerSupport()
{
	unsigned int layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	for (const char* layerName : mValidationLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if(strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				return true;
			}
		}
	}
	return false;
}
/// Busqueda de la Addr para la funcion cargada vkCreateDebugUtilsMessengerEXT
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkCreateDebugUtilsMessengerEXT");
	if(function != nullptr)
	{
		return function(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		fprintf(stderr, "Not found vkCreateDebugUtilsMessengerEXT\n");
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
	VkDebugUtilsMessengerEXT debugMessenger, 
	const VkAllocationCallbacks* pAllocator)
{
	auto function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkDestroyDebugUtilsMessengerEXT");
	if (function != nullptr)
	{
		return function(instance, debugMessenger, pAllocator);
	}
}

/// Callback para la capa de validacion de Debug 
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	switch (messageType)
	{
		// Un evento no relacionado con la spec y el rendimiento
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		// Evento que incumple la especificacion y puede provocar errores.
		//fprintf(stderr, "\tMessage Type: VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT\n");
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		// Uso no optimo de la API de Vulkan.
		//fprintf(stderr, "\tMessage Type: VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT\n");
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		fprintf(stderr, "\tMessage Type: VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT\n");
		break;
	default:
		break;
	}
	switch(messageSeverity)
	{
			// Mensaje diagnositco
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			break;
			// mensaje informativo
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			break;
			// Mesaje de comportamiento que puede ser un BUG en la app
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			break;
			// Mensaje sobre un comportamiento invalido y que puede provocar CRASH
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			fprintf(stderr, "\tVLayer message: %s\n", pCallbackData->pMessage);
			break;
		default:
			break;
	}
	return VK_FALSE;
}

void CleanSwapChain()
{
	for (auto& framebuffer : mSwapChainFramebuffers)
		vkDestroyFramebuffer(mLogicDevice, framebuffer, nullptr);
	for (auto& imageView : mSwapChainImagesViews)
		vkDestroyImageView(mLogicDevice, imageView, nullptr);
	vkDestroySwapchainKHR(mLogicDevice, mSwapChain, nullptr);
}

void Cleanup()
{
	printf("Cleanup\n");
	vkDeviceWaitIdle(mLogicDevice);
	vkDestroyBuffer(mLogicDevice, mIndexBuffer, nullptr);
	vkFreeMemory(mLogicDevice, mIndexBufferMemory, nullptr);
	vkDestroyBuffer(mLogicDevice, mVertexBuffer, nullptr);
	vkFreeMemory(mLogicDevice, mVertexBufferMemory, nullptr);

	vkDestroySemaphore(mLogicDevice, mImageAvailable[0], nullptr);
	vkDestroySemaphore(mLogicDevice, mImageAvailable[1], nullptr);
	vkDestroySemaphore(mLogicDevice, mRenderFinish[0], nullptr);
	vkDestroySemaphore(mLogicDevice, mRenderFinish[1], nullptr);
	vkDestroyFence(mLogicDevice, mInFlight[0], nullptr);
	vkDestroyFence(mLogicDevice, mInFlight[1], nullptr);
	vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);
	vkDestroyPipeline(mLogicDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mLogicDevice, mPipelineLayout, nullptr);
	vkDestroyRenderPass(mLogicDevice, mRenderPass, nullptr);
	CleanSwapChain();
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyDevice(mLogicDevice, nullptr);
	if (mDebugMessenger != nullptr)
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	vkDestroyInstance(mInstance, nullptr);
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void InitializeVulkan(VkApplicationInfo* _appInfo)
{
	_appInfo->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	_appInfo->pApplicationName = "Hello, Sailor";
	_appInfo->applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	_appInfo->pEngineName = "Raisin";
	_appInfo->engineVersion = VK_MAKE_VERSION(1, 0, 0);
	_appInfo->apiVersion = VK_API_VERSION_1_3;
}

void CreateInstance(VkInstanceCreateInfo* _createInfo, VkApplicationInfo* _appInfo, const char** m_Extensions,
	uint32_t m_extensionCount)
{
	_createInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	_createInfo->pApplicationInfo = _appInfo;
	_createInfo->enabledExtensionCount = m_extensionCount;
	_createInfo->ppEnabledExtensionNames = mInstanceExtensions.data();
	_createInfo->enabledLayerCount = mValidationLayers.size();
	_createInfo->ppEnabledLayerNames = mValidationLayers.data();
}

void CreateLogicalDevice(VkPhysicalDevice* _phisicaldevice, 
	VkPhysicalDeviceFeatures* _deviceFeatures)
{
	VkDeviceCreateInfo mCreateInfo{};
	mCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	mCreateInfo.pQueueCreateInfos = mQueueCreateInfos.data();
	mCreateInfo.queueCreateInfoCount = mQueueCreateInfos.size();
	mCreateInfo.pEnabledFeatures = _deviceFeatures;
	mCreateInfo.enabledExtensionCount = mDeviceExtensions.size();
	mCreateInfo.ppEnabledExtensionNames = mDeviceExtensions.data();
	mCreateInfo.enabledLayerCount = 0;

	if (vkCreateDevice(*_phisicaldevice, &mCreateInfo, nullptr, &mLogicDevice) !=
		VK_SUCCESS)
	{
		printf("Failed to create Logical Device");
		Cleanup();
		exit(-1);
	}
}

void CreateSwapChain()
{
	mSwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	mSwapChainCreateInfo.surface = mSurface;
	mSwapChainCreateInfo.minImageCount = mSwapchainImagesCount;
	mSwapChainCreateInfo.imageFormat = mSurfaceFormat.format;
	mSwapChainCreateInfo.imageColorSpace = mSurfaceFormat.colorSpace;
	mSwapChainCreateInfo.imageExtent = mCurrentExtent;
	mSwapChainCreateInfo.imageArrayLayers = 1;
	//VK_IMAGE_USAGE_TRANSFER_DST_BIT 
	mSwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	mSwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//_swapChainCreateInfo->pQueueFamilyIndices = mgr;
	mSwapChainCreateInfo.preTransform = mCapabilities.currentTransform;
	mSwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	mSwapChainCreateInfo.presentMode = mPresentMode;
	mSwapChainCreateInfo.clipped = VK_TRUE;
	mSwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(mLogicDevice, &mSwapChainCreateInfo, nullptr, &mSwapChain) != VK_SUCCESS)
		exit(-3);
}

void CreateImageViews()
{
	/// Ahora vamos a crear las vistas a la imagenes, para poder acceder a ellas y demas
	mSwapChainImagesViews.resize(mSwapchainImagesCount);
	int currentSwapchaingImageView = 0;
	for (auto& image : mSwapChainImages)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mSurfaceFormat.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(mLogicDevice, &imageViewCreateInfo, nullptr,
			&mSwapChainImagesViews[currentSwapchaingImageView]) != VK_SUCCESS)
			exit(-4);
		++currentSwapchaingImageView;
	}
}

void CreateShaderModule(const char* _shaderPath, VkShaderModule* _shaderModule)
{
	char shaderPath[64];
	strcpy(shaderPath, _shaderPath);
	auto shadercode = LoadShader(shaderPath);
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = shadercode.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shadercode.data());
	if (vkCreateShaderModule(mLogicDevice, &shaderModuleCreateInfo, nullptr, _shaderModule)
		!= VK_SUCCESS)
		exit(-5);
}

void CreatePipelineLayout(VkShaderModule* _vertShaderModule, VkShaderModule* _fragShaderModule, 
	VkPipelineShaderStageCreateInfo* _shaderStages,		VkPipelineInputAssemblyStateCreateInfo* mInputAssembly,
	VkPipelineDynamicStateCreateInfo* mDynamicState,		VkPipelineVertexInputStateCreateInfo* mVertexInputInfo,
	VkPipelineViewportStateCreateInfo* mViewportState,	
	VkPipelineRasterizationStateCreateInfo* mRasterizer, VkPipelineMultisampleStateCreateInfo* mMultisampling, 
	VkPipelineDepthStencilStateCreateInfo* mDepthStencil, VkPipelineColorBlendStateCreateInfo* mColorBlending)
{
	/// Creacion de los shader stage de la Pipeline
	VkPipelineShaderStageCreateInfo mVertShaderStageInfo{};
	mVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	mVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	mVertShaderStageInfo.module = *_vertShaderModule;
	mVertShaderStageInfo.pName = "main";
	VkPipelineShaderStageCreateInfo mFragShaderStageInfo{};
	mFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	mFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	mFragShaderStageInfo.module = *_fragShaderModule;
	mFragShaderStageInfo.pName = "main";

	_shaderStages[0] = mVertShaderStageInfo;
	_shaderStages[1] = mFragShaderStageInfo;
	/// Dynamic State (stados que permiten ser cambiados sin re-crear toda la pipeline)
	mDynamicState->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	mDynamicState->dynamicStateCount = static_cast<uint32_t>(mDynamicStates.size());
	mDynamicState->pDynamicStates = mDynamicStates.data();
	/// Vertex Input (los datos que l epasamos al shader per-vertex o per-instance)
	mVertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	mVertexInputInfo->vertexBindingDescriptionCount = 1;
	mVertexInputInfo->vertexAttributeDescriptionCount = static_cast<unsigned int>(mAttributeDescriptions.size());
	mVertexInputInfo->pVertexBindingDescriptions = &mBindingDescription;
	mVertexInputInfo->pVertexAttributeDescriptions = mAttributeDescriptions.data();
	/// Definimos la geometria que vamos a pintar
	mInputAssembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	mInputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	mInputAssembly->primitiveRestartEnable = VK_FALSE;
	/// Definimos el Viewport de la app
	mViewport.x = 0.f;
	mViewport.y = 0.f;
	mViewport.width = mCurrentExtent.width;
	mViewport.height = mCurrentExtent.height;
	mViewport.minDepth = 0.f;
	mViewport.maxDepth = 1.f;
	/// definamos el Scissor Rect de la app
	mScissor.offset = { 0, 0 };
	mScissor.extent = mCurrentExtent;
	mViewportState->viewportCount = 1;
	mViewportState->scissorCount = 1;
	mViewportState->pViewports = &mViewport;
	mViewportState->pScissors = &mScissor;
	/* Si no estuvieramos en modo Dynamico, necesitariamos establecer la
	 * informacion de creacion del ViewportState y los Vewport punteros.
	 */
	 /// El rasterizador convierte los vertices en fragmentos para darles color
	mRasterizer->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	mRasterizer->depthClampEnable = VK_FALSE;
	mRasterizer->rasterizerDiscardEnable = VK_FALSE;
	mRasterizer->polygonMode = VK_POLYGON_MODE_FILL;
	mRasterizer->lineWidth = 1.f;
	mRasterizer->cullMode = VK_CULL_MODE_BACK_BIT;
	mRasterizer->frontFace = VK_FRONT_FACE_CLOCKWISE;
	mRasterizer->depthBiasEnable = VK_FALSE;
	/// Multisampling para evitar los bordes de sierra (anti-aliasing).
	mMultisampling->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	mMultisampling->sampleShadingEnable = VK_FALSE;
	mMultisampling->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	mMultisampling->minSampleShading = 1.f;
	mMultisampling->pSampleMask = nullptr;
	mMultisampling->alphaToCoverageEnable = VK_FALSE;
	mMultisampling->alphaToOneEnable = VK_FALSE;
	/// Depth and stencil
	mDepthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	/// Pipeline Layout
	VkPipelineLayoutCreateInfo mPipelineLayoutCreateInfo{};
	mPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	mPipelineLayoutCreateInfo.setLayoutCount = 0;
	mPipelineLayoutCreateInfo.pSetLayouts = nullptr;
	mPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	mPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(mLogicDevice, &mPipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		exit(-7);
}

void CreateRenderPass(VkSwapchainCreateInfoKHR* mSwapChainCreateInfo)
{
	// RENDER PASES
	/// Attachment description
	VkAttachmentDescription mColorAttachment{};
	mColorAttachment.format = mSwapChainCreateInfo->imageFormat;
	mColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	mColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	mColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	mColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	mColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// SUB-PASSES
	/// Attachment References
	VkAttachmentReference mColorAttachmentRef{};
	mColorAttachmentRef.attachment = 0;
	mColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	/// Sub-pass
	VkSubpassDescription mSubpass{};
	mSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	mSubpass.colorAttachmentCount = 1;
	mSubpass.pColorAttachments = &mColorAttachmentRef;
	/// Subpass dependencies
	VkSubpassDependency mSubpassDep{};
	mSubpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
	mSubpassDep.dstSubpass = 0;
	mSubpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	mSubpassDep.srcAccessMask = 0;
	mSubpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	mSubpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	/// Render pass
	VkRenderPassCreateInfo mRenderPassInfo{};
	mRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	mRenderPassInfo.attachmentCount = 1;
	mRenderPassInfo.pAttachments = &mColorAttachment;
	mRenderPassInfo.subpassCount = 1;
	mRenderPassInfo.pSubpasses = &mSubpass;
	mRenderPassInfo.dependencyCount = 1;
	mRenderPassInfo.pDependencies = &mSubpassDep;
	if (vkCreateRenderPass(mLogicDevice, &mRenderPassInfo, nullptr,
		&mRenderPass) != VK_SUCCESS)
		exit(-8);
}


void CopyBuffer(VkBuffer dst_, VkBuffer _src, VkDeviceSize _size)
{
	VkCommandBufferAllocateInfo mAllocInfo{};
	mAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	mAllocInfo.commandPool = mCommandPool;
	mAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	mAllocInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(mLogicDevice, &mAllocInfo, &commandBuffer) != VK_SUCCESS)
		exit(-12);
	VkCommandBufferBeginInfo mBeginInfo{};
	mBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	mBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(commandBuffer, &mBeginInfo) != VK_SUCCESS)
		exit(-13);
	// Copiar desde el Stagging buffer al buffer
	VkBufferCopy copyRegion {};
	copyRegion.size = _size;
	vkCmdCopyBuffer(commandBuffer, _src, dst_, 1, &copyRegion);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		exit(-17);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mGraphicsQueue);
	vkFreeCommandBuffers(mLogicDevice, mCommandPool, 1, &commandBuffer);
}

void RecordCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIdx, unsigned int _frameIdx)
{
	// Record command buffer
	VkCommandBufferBeginInfo mBeginInfo{};
	mBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	mBeginInfo.flags = 0;
	mBeginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(_commandBuffer, &mBeginInfo) != VK_SUCCESS)
		exit(-13);
	// Render pass
	VkRenderPassBeginInfo mRenderPassInfo{};
	mRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	mRenderPassInfo.renderPass = mRenderPass;
	mRenderPassInfo.framebuffer = mSwapChainFramebuffers[_imageIdx];
	mRenderPassInfo.renderArea.offset = { 0,0 };
	mRenderPassInfo.renderArea.extent = mCurrentExtent;
	// Clear Color
	VkClearValue mClearColor = { {{0.f, 0.f, 0.f, 1.f}} };
	mRenderPassInfo.clearValueCount = 1;
	mRenderPassInfo.pClearValues = &mClearColor;
	vkCmdBeginRenderPass(_commandBuffer, &mRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	// Drawing Commands
	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
	vkCmdSetViewport(_commandBuffer, 0, 1, &mViewport);
	vkCmdSetScissor(_commandBuffer, 0, 1, &mScissor);
	VkBuffer vertesBuffers[] = {mVertexBuffer};
	VkBuffer indexBuffer = mIndexBuffer;
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertesBuffers, offsets);
	vkCmdBindIndexBuffer(_commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	//vkCmdDraw(_commandBuffer, 3, 1, 0, 0);
	vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(mIndices.size()),
					 1, 0, 0, 0);
	vkCmdEndRenderPass(_commandBuffer);
	if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
		exit(-17);
}

void CreateFramebuffers()
{
	// FRAMEBUFFERS
	mSwapChainFramebuffers.resize(mSwapChainImagesViews.size());
	for (size_t i = 0; i < mSwapChainImagesViews.size(); i++)
	{
		VkImageView attachments[] = {
			mSwapChainImagesViews[i]
		};
		VkFramebufferCreateInfo mFramebufferInfo{};
		mFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		mFramebufferInfo.renderPass = mRenderPass;
		mFramebufferInfo.attachmentCount = 1;
		mFramebufferInfo.pAttachments = attachments;
		mFramebufferInfo.width = mCurrentExtent.width;
		mFramebufferInfo.height = mCurrentExtent.height;
		mFramebufferInfo.layers = 1;
		if (vkCreateFramebuffer(mLogicDevice, &mFramebufferInfo, nullptr,
			&mSwapChainFramebuffers[i]) != VK_SUCCESS)
			exit(-10);
	}
}

void CreateCommandBuffer()
{
	// COMMAND BUFFERS
	/// Command Pool
	///	mGraphicsQueueFamilyIndex
	VkCommandPoolCreateInfo mPoolInfo{};
	mPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	mPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	mPoolInfo.queueFamilyIndex = mGraphicsQueueFamilyIndex;
	if (vkCreateCommandPool(mLogicDevice, &mPoolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		exit(-11);
	VkCommandBufferAllocateInfo mAllocInfo{};
	mAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	mAllocInfo.commandPool = mCommandPool;
	mAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	mAllocInfo.commandBufferCount = 2;
	if (vkAllocateCommandBuffers(mLogicDevice, &mAllocInfo, mCommandBuffer) != VK_SUCCESS)
		exit(-12);
}

void CreateSyncObjects(unsigned int _frameIdx)
{
	// Creamos los elementos de sincronizacion
	VkSemaphoreCreateInfo mSemaphoreInfo{};
	mSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo mFenceInfo{};
	mFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	mFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateSemaphore(mLogicDevice, &mSemaphoreInfo, nullptr, &mImageAvailable[_frameIdx]) != VK_SUCCESS
		|| vkCreateSemaphore(mLogicDevice, &mSemaphoreInfo, nullptr, &mRenderFinish[_frameIdx]) != VK_SUCCESS
		|| vkCreateFence(mLogicDevice, &mFenceInfo, nullptr, &mInFlight[_frameIdx]) != VK_SUCCESS
		)
		exit(-666);
}

void RecreateSwapChain()
{
	printf("\n\tRe-create Swapchain\n");
	// Si estamos minimizados, esperamos pacientemente a que se vuelva a ver la ventana
	int width = 0, height = 0;
	glfwGetFramebufferSize(mWindow, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(mWindow, &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(mLogicDevice);
	// Esperamos a que termine de pintarse y recreamos la swapchain con los nuevos parametros
	CleanSwapChain();
	CreateSwapChain();
	vkGetSwapchainImagesKHR(mLogicDevice, mSwapChain, &mSwapchainImagesCount, nullptr);
	mSwapChainImages.resize(mSwapchainImagesCount);
	vkGetSwapchainImagesKHR(mLogicDevice, mSwapChain, &mSwapchainImagesCount, mSwapChainImages.data());
	CreateImageViews();
	CreateFramebuffers();
	mNeedToRecreateSwapchain = false;
}
/* Un Frame en Vulkan
 *  1. esperar a que el frame anterior acabe
 *	2. obtener la imagen de la swap chain
 *	3. grabar el command buffer que pinta la escena en la imagen
 *	4. Submit del command buffer
 *	5. presentar la imagen de la swap chain
 */
/* Sincronizacion GPU & CPU
	Semaforos: (solo afectan a GPU)
		anadir orden entre operaciones de la cola(trabajo que encolamos).
		Ejemplos de colas: cola de graficos y cola de presentacion.
		Los semaforos sirven para ordenar trabajo dentro de las colas y entre colas
		2 tipos de semaforos: binarios y de timeline

	Vallas: son igual que los semaforos pero estos sirven para ordenar ejecucion
		entre la CPU y GPU. Los usamos cuando la CPU debe esperar a que acabe la GPU
		para continuar.
*/ 
void DrawFrame()
{
	// Esperamos por el frame anterior y reseteamos la Fence
	vkWaitForFences(mLogicDevice, 1, &mInFlight[mCurrentLocalFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(mLogicDevice, 1, &mInFlight[mCurrentLocalFrame]);
	vkResetCommandBuffer(mCommandBuffer[mCurrentLocalFrame], 0);
	uint32_t imageIdx;
	vkAcquireNextImageKHR(mLogicDevice, mSwapChain, UINT64_MAX, mImageAvailable[mCurrentLocalFrame],
		VK_NULL_HANDLE, &imageIdx);
	RecordCommandBuffer(mCommandBuffer[mCurrentLocalFrame], imageIdx, mCurrentLocalFrame);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = {mImageAvailable[mCurrentLocalFrame] };
	VkPipelineStageFlags waitStages[] ={VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffer[mCurrentLocalFrame];
	VkSemaphore signalSemaphores[] = {mRenderFinish[mCurrentLocalFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlight[mCurrentLocalFrame]) != VK_SUCCESS)
	{
		fprintf(stderr, "Error on the Submit");
		exit(-1);
	}
	// Presentacion: devolver el frame al swapchain para que salga por pantalla
	VkPresentInfoKHR presentInfo {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = {mSwapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIdx;
	presentInfo.pResults = nullptr;
	mPresentResult = vkQueuePresentKHR(mPresentQueue, &presentInfo);
}

void FramebufferResizeCallback(GLFWwindow* _window, int _newW, int _newH)
{
	mNeedToRecreateSwapchain = true;
}

unsigned int FindMemoryType(unsigned int typeFilter, VkMemoryPropertyFlags properties)
{
	for(unsigned int i = 0; i < mMemProps.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (mMemProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			printf("\nmemType %d\n", i);
			return i;
		}
	}
	exit(-9);
}

void CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode, VkMemoryPropertyFlags _memFlags,VkBuffer& buffer_,
				  VkDeviceMemory& bufferMem_)
{
	/*
		Create Vertex Buffers
	 */
	unsigned int queueFamilyIndices[] = {mGraphicsQueueFamilyIndex, mTransferQueueFamilyIndex};
	VkBufferCreateInfo mBufferInfo {};
	mBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	mBufferInfo.size = _size;
	mBufferInfo.usage = _usage;
	mBufferInfo.sharingMode = _shareMode;
	mBufferInfo.pQueueFamilyIndices = queueFamilyIndices;
	mBufferInfo.queueFamilyIndexCount = 2;

	if(vkCreateBuffer(mLogicDevice, &mBufferInfo, nullptr, &buffer_) != VK_SUCCESS)
		exit(-6);
	VkMemoryRequirements memRequ;
	vkGetBufferMemoryRequirements(mLogicDevice, buffer_, &memRequ);

	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevices[mGPUSelected], &mMemProps);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequ.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequ.memoryTypeBits, _memFlags);
	if(vkAllocateMemory(mLogicDevice, &allocInfo, nullptr, &bufferMem_) != VK_SUCCESS)
		exit(-8);

	vkBindBufferMemory(mLogicDevice, buffer_, bufferMem_, 0);

}

int main()
{
	uint32_t mExtensionCount = 0;
	const char** mExtensions;
	glm::mat4 m_matrix;
	glm::vec4 m_vec;
	const int m_Width = 800;
	const int m_Height = 600;

	/// VULKAN THINGS
	if (!checkValidationLayerSupport()) return -2;
	VkApplicationInfo mAppInfo = {};
	InitializeVulkan(&mAppInfo);
	// Fill Extension supported by GPU
	mExtensions = glfwGetRequiredInstanceExtensions(&mExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &mExtensionCount, nullptr);
	std::vector<VkExtensionProperties> mExtensionsProps(mExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &mExtensionCount, mExtensionsProps.data());
	for (const auto& ext : mExtensionsProps)
	{
		mInstanceExtensions.push_back(ext.extensionName);
	}
	mInstanceExtensions.push_back("VK_EXT_debug_utils");

	//mInstanceExtensions.push_back("VK_KHR_swapchain");
	
	VkInstanceCreateInfo mInstanceCreateInfo = {};
	CreateInstance( &mInstanceCreateInfo, &mAppInfo, mExtensions, mExtensionCount );
	VkResult m_result = vkCreateInstance(&mInstanceCreateInfo, nullptr, &mInstance);
	if(m_result != VK_SUCCESS)
	{
		printf("ERROR CREATING INSTANCE VULKAN");
		return -1;
	}
	/// NORMAL RENDER THINGS
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	mWindow = glfwCreateWindow(m_Width, m_Height, "Vulkan test", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
	/// Create the Debug Messenger
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;
	debugCreateInfo.pUserData = nullptr;
	if(CreateDebugUtilsMessengerEXT(mInstance, &debugCreateInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
	{
		fprintf(stderr, "ERROR CREATING DEBUG MESSENGER\n");
	}

	/// Asociamos el VkDevice a la GPU
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		fprintf(stderr, "Not GPU FOUND\n");
		exit(-1);
	}
	mPhysicalDevices.resize(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, mPhysicalDevices.data());
	VkPhysicalDeviceProperties deviceProp[deviceCount];
	VkPhysicalDeviceFeatures deviceFeatures[deviceCount];
	mGPUSelected = 0;
	for(int it = 0; it < deviceCount; it++)
	{
		vkGetPhysicalDeviceProperties(mPhysicalDevices[it], &deviceProp[it]);
		if(strstr(deviceProp[it].deviceName, "NVIDIA") || strstr(deviceProp[it].deviceName,"AMD"))
		{
			mGPUSelected = it;
			printf("\nGPU %d: %s\n", it, deviceProp[mGPUSelected].deviceName);
		}
	vkGetPhysicalDeviceFeatures(mPhysicalDevices[it], &deviceFeatures[it]);
	}
	/// Comprobamos que tengamos la extension VK_KHR_swapchain soportada en el device GPU
	unsigned int deviceExtensionCount;
	vkEnumerateDeviceExtensionProperties(mPhysicalDevices[mGPUSelected], nullptr, &deviceExtensionCount, nullptr);
	std::vector<VkExtensionProperties> deviceAvailableExtensions(deviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(mPhysicalDevices[mGPUSelected], nullptr, &deviceExtensionCount, deviceAvailableExtensions.data());
	for (const auto& ext : deviceAvailableExtensions)
	{
		// VK_KHR_buffer_device_address and VK_EXT_buffer_device_address not at the same time
		if (strcmp(ext.extensionName, "VK_EXT_buffer_device_address") == 0)
			continue;
		mDeviceExtensions.push_back(ext.extensionName);
	}
	/// Buscamos las familias de Colas.
	unsigned int queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevices[mGPUSelected], &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevices[mGPUSelected], &queueFamilyCount, queueFamilies.data());
	// VK_QUEUE_GRAPHICS_BIT
	bool searchingGraphics = true, searchingTransfer = true;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			printf("\nGraphics Family: %d\n", mTransferQueueFamilyIndex);
			searchingGraphics = false;
		}
		if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			printf("\nTransfer Family: %d\n", mTransferQueueFamilyIndex);
			searchingTransfer = false;
		}

		mGraphicsQueueFamilyIndex += searchingGraphics;
		mTransferQueueFamilyIndex += searchingTransfer;
		if(!searchingGraphics && !searchingTransfer)
			break;
	}

	/// Ahora vamos a crear el device logico para interactuar con él
	float queuePriority = 1.f;

	mQueueCreateInfos.push_back({
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = mGraphicsQueueFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
		});

	mQueueCreateInfos.push_back({
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = mTransferQueueFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
		});

	// Create logical device associated to physical device
	CreateLogicalDevice(&mPhysicalDevices[mGPUSelected], &deviceFeatures[mGPUSelected]);
	vkGetDeviceQueue(mLogicDevice, mGraphicsQueueFamilyIndex, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mLogicDevice, mTransferQueueFamilyIndex, 0, &mTransferQueue);

	/// Vamos a crear la integracion del sistema de ventanas (WSI) para vulkan
	// EXT: VK_KHR_surface
	if(glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
		exit(-2);

	// Present support on the Physical Device
	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevices[mGPUSelected], mGraphicsQueueFamilyIndex, mSurface, &presentSupport);
	if (!presentSupport)
		printf("CANNOT PRESENT ON THIS DEVICE: %s\n", deviceProp[mGPUSelected].deviceName);
	vkGetDeviceQueue(mLogicDevice, mGraphicsQueueFamilyIndex, 0, &mPresentQueue);

	/* Ahora tenemos que ver que nuestro device soporta la swapchain
	 * y sus capacidades.
	*/
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevices[mGPUSelected], mSurface, &mCapabilities);
	unsigned int surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevices[mGPUSelected], mSurface, &surfaceFormatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevices[mGPUSelected], mSurface, &surfaceFormatCount, surfaceFormats.data());
	unsigned int presentModesCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevices[mGPUSelected], mSurface, &presentModesCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevices[mGPUSelected], mSurface, &presentModesCount, presentModes.data());
	/// Ahora elegimos el formato que cumpla nuestras necesidades.
	unsigned int formatChoosen = 0;
	for(const auto& format : surfaceFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			mSurfaceFormat = surfaceFormats[formatChoosen];
			break;
		}
		++formatChoosen;
	}
	/// Elegimos el modo de presentacion a la pantalla
	///	VK_PRESENT_MODE_IMMEDIATE_KHR
	/// VK_PRESENT_MODE_FIFO_KHR (el unico que nos aseguran que este disponible)
	/// VK_PRESENT_MODE_FIFO_RELAXED_KHR
	/// VK_PRESENT_MODE_MAILBOX_KHR
	for (const auto& presentMode : presentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			mPresentMode = presentMode;
			break;
		}
	}
	/// ahora vamos a establecer el swap extent, que es la resolucion de las imagnes de la swapchain
	mCurrentExtent = mCapabilities.currentExtent;
	/// Ahora creamos la swapchain como tal.
	mSwapchainImagesCount = mCapabilities.minImageCount;

	CreateSwapChain();
	vkGetSwapchainImagesKHR(mLogicDevice, mSwapChain, &mSwapchainImagesCount, nullptr);
	mSwapChainImages.resize(mSwapchainImagesCount);
	vkGetSwapchainImagesKHR(mLogicDevice, mSwapChain, &mSwapchainImagesCount, mSwapChainImages.data());
	CreateImageViews();

	/// Vamos a crear los shader module para cargar el bytecode de los shaders
	VkShaderModule mVertShaderModule;
	VkShaderModule mFragShaderModule;
	CreateShaderModule("resources/Shaders/vert.spv", &mVertShaderModule);
	CreateShaderModule("resources/Shaders/frag.spv", &mFragShaderModule);

	// Create Pipeline Layout
	VkPipelineShaderStageCreateInfo mShaderStages[2] = {};
	VkPipelineInputAssemblyStateCreateInfo mInputAssembly{};
	VkPipelineDynamicStateCreateInfo mDynamicState{};
	VkPipelineVertexInputStateCreateInfo mVertexInputInfo{};
	VkPipelineViewportStateCreateInfo mViewportState{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
	};
	VkPipelineRasterizationStateCreateInfo mRasterizer{};
	VkPipelineMultisampleStateCreateInfo mMultisampling{};
	VkPipelineDepthStencilStateCreateInfo mDepthStencil{};
	VkPipelineColorBlendStateCreateInfo mColorBlending{};
	/// Color Blending
	VkPipelineColorBlendAttachmentState mColorBlendAttachment{};
	mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	mColorBlendAttachment.blendEnable = VK_FALSE;
	mColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	mColorBlending.logicOpEnable = VK_FALSE;
	mColorBlending.attachmentCount = 1;
	mColorBlending.pAttachments = &mColorBlendAttachment;

	CreatePipelineLayout(&mVertShaderModule, &mFragShaderModule, mShaderStages, &mInputAssembly, &mDynamicState, &mVertexInputInfo,&mViewportState, &mRasterizer,
		&mMultisampling, &mDepthStencil, &mColorBlending);
	CreateRenderPass(&mSwapChainCreateInfo);

	/// Create Graphics pipeline
	VkGraphicsPipelineCreateInfo mPipelineInfoCreateInfo {};
	mPipelineInfoCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	mPipelineInfoCreateInfo.stageCount = 2;
	mPipelineInfoCreateInfo.pStages = mShaderStages;
	mPipelineInfoCreateInfo.pVertexInputState = &mVertexInputInfo;
	mPipelineInfoCreateInfo.pInputAssemblyState = &mInputAssembly;
	mPipelineInfoCreateInfo.pViewportState = &mViewportState;
	mPipelineInfoCreateInfo.pRasterizationState = &mRasterizer;
	mPipelineInfoCreateInfo.pMultisampleState = &mMultisampling;
	mPipelineInfoCreateInfo.pDepthStencilState = nullptr;
	mPipelineInfoCreateInfo.pColorBlendState = &mColorBlending;
	mPipelineInfoCreateInfo.pDynamicState = &mDynamicState;
	mPipelineInfoCreateInfo.layout = mPipelineLayout;
	mPipelineInfoCreateInfo.renderPass = mRenderPass;
	mPipelineInfoCreateInfo.subpass = 0;
	mPipelineInfoCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	mPipelineInfoCreateInfo.basePipelineIndex = -1;
	mGraphicsPipeline = VkPipeline {};

	if (vkCreateGraphicsPipelines(mLogicDevice, VK_NULL_HANDLE, 1, &mPipelineInfoCreateInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
		exit(-9);
	// Destruymos los ShaderModule ahora que ya no se necesitan.
	vkDestroyShaderModule(mLogicDevice, mVertShaderModule, nullptr);
	vkDestroyShaderModule(mLogicDevice, mFragShaderModule, nullptr);

	CreateFramebuffers();
	CreateCommandBuffer();

	// Vertex buffer
	VkDeviceSize bufferSize = sizeof(mTriangleVertices[0]) * mTriangleVertices.size();
	// Stagin buffer
	CreateBuffer(bufferSize,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			  mStagingBuffer, mStaggingBufferMemory);
	void* data;
	vkMapMemory(mLogicDevice, mStaggingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mTriangleVertices.data(), (size_t) bufferSize);
	vkUnmapMemory(mLogicDevice, mStaggingBufferMemory);
	CreateBuffer(bufferSize,
				 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_SHARING_MODE_CONCURRENT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			  mVertexBuffer, mVertexBufferMemory);
	CopyBuffer(mVertexBuffer, mStagingBuffer, bufferSize);
	vkDestroyBuffer(mLogicDevice, mStagingBuffer, nullptr);
	vkFreeMemory(mLogicDevice, mStaggingBufferMemory, nullptr);

	// Index buffer
	bufferSize = sizeof(mIndices[0]) * mIndices.size();
	// Stagin buffer
	CreateBuffer(bufferSize,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			  mStagingBuffer, mStaggingBufferMemory);
	vkMapMemory(mLogicDevice, mStaggingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mIndices.data(), (size_t) bufferSize);
	vkUnmapMemory(mLogicDevice, mStaggingBufferMemory);
	CreateBuffer(bufferSize,
				 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 VK_SHARING_MODE_CONCURRENT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			  mIndexBuffer, mIndexBufferMemory);
	CopyBuffer(mIndexBuffer, mStagingBuffer, bufferSize);
	vkDestroyBuffer(mLogicDevice, mStagingBuffer, nullptr);
	vkFreeMemory(mLogicDevice, mStaggingBufferMemory, nullptr);

	RecordCommandBuffer(mCommandBuffer[0], 0, 0);
	RecordCommandBuffer(mCommandBuffer[1], 0, 1);
	CreateSyncObjects(0);
	CreateSyncObjects(1);

	while (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
		// Draw a Frame!
		DrawFrame();
// 		if ((mPresentResult == VK_ERROR_OUT_OF_DATE_KHR || mPresentResult == VK_SUBOPTIMAL_KHR)
// 			&& mNeedToRecreateSwapchain)
// 			RecreateSwapChain();
// 		else if (mPresentResult != VK_SUCCESS && mPresentResult != VK_SUBOPTIMAL_KHR)
// 			exit(-69);
		mCurrentLocalFrame = (mCurrentLocalFrame + 1) % FRAMES_IN_FLIGHT;
	}
	Cleanup();

	return 0;
}

