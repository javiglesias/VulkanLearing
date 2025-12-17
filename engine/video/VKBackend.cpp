#include "VKBackend.h"
#include "VKDevice.h"
#include "../filesystem/ResourceManager.h"
#include "glslang/Public/ShaderLang.h"
#include "../core/Materials/VKRTexture.h"
#include "../memory/mem_alloc.h"
#include "VKRUtils.h"
#include "../input/DeviceInput.h"
#include "../perfmon/Custom.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#ifndef VMA_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "../../../dependencies/stb/stb_image.h"

#ifndef WIN32
#include <signal.h>
#endif
#ifndef USE_GLFW
#include <vulkan/vulkan_win32.h>
bool should_close_window = false;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		return 0;
	case WM_LBUTTONDOWN:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	case WM_MOUSEMOVE:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 1;
}
#else
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

namespace VKR
{
    namespace render
    {
		void FramebufferResizeCallback(GLFWwindow* _window, int _newW, int _newH)
		{
			m_NeedToRecreateSwapchain = true;
		}

		static VKBackend g_backend;
		VKBackend& GetVKBackend()
		{
			return g_backend;
		}

		/// Busqueda de la Addr para la funcion cargada vkCreateDebugUtilsMessengerEXT
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
				"vkCreateDebugUtilsMessengerEXT");
			if (function != nullptr)
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
				break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
				// Uso no optimo de la API de Vulkan.
				break;
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
				ChangeColorConsole(CONSOLE_COLOR::NORMAL);
				fprintf(stderr, "\tMessage PERFORMANCE: %s\n", pCallbackData->pMessage);
				break;
			default:
				break;
			}
			switch (messageSeverity)
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
#ifdef WIN32
				ChangeColorConsole(CONSOLE_COLOR::RED);
				fprintf(stderr, "\n\tVLayer message: %s\n", pCallbackData->pMessage);
				ChangeColorConsole(CONSOLE_COLOR::NORMAL);
#else
				fprintf(stderr, "\033[1;31m\n\tVLayer message: %s\033[0m\n", pCallbackData->pMessage);
#endif
				break;
			default:
				break;
			}
			return VK_FALSE;
		}

		void VKBackend::Init(
#ifndef USE_GLFW
			HINSTANCE hInstance
#endif
		)
		{
#ifndef USE_GLFW
			init_input_devices();
#endif
			#ifdef WIN32
			glslang::InitializeProcess();
			printf("glslang GLSL version: %s\n", glslang::GetGlslVersionString());
			#endif
			m_GPipelineStatus = CREATING;
			uint32_t mExtensionCount = 0;
			/// VULKAN THINGS
			if (!g_context.m_GpuInfo.CheckValidationLayerSupport()) exit(-2);
			VkApplicationInfo mAppInfo = {};
			InitializeVulkan(&mAppInfo);
			// Fill Extension supported by GPU
			vkEnumerateInstanceExtensionProperties(nullptr, &mExtensionCount, nullptr);
			std::vector<VkExtensionProperties> mExtensionsProps(mExtensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &mExtensionCount, mExtensionsProps.data());
			for (const auto& ext : mExtensionsProps)
			{
				m_InstanceExtensions.push_back(ext.extensionName);
			}
			m_InstanceExtensions.push_back("VK_EXT_debug_utils");
			m_InstanceExtensions.push_back("VK_KHR_dedicated_allocation");
			m_InstanceExtensions.push_back("VK_KHR_bind_memory2");
			m_InstanceExtensions.push_back("VK_KHR_maintenance4");

			//m_InstanceExtensions.push_back("VK_KHR_swapchain");

			VkInstanceCreateInfo m_InstanceCreateInfo = {};
			CreateInstance(&m_InstanceCreateInfo, &mAppInfo, mExtensionCount);
			VkResult m_result = vkCreateInstance(&m_InstanceCreateInfo, nullptr, &m_Instance);
			if (m_result != VK_SUCCESS)
			{
				printf("ERROR CREATING INSTANCE VULKAN");
				exit(- 1);
			}
			/// NORMAL RENDER THINGS
#ifdef USE_GLFW
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			m_Window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "Vulkan renderer", nullptr, nullptr);
			glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
			glfwSetKeyCallback(m_Window, KeyboardInputCallback);
			glfwSetCursorPosCallback(m_Window, MouseCallback);
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetMouseButtonCallback(m_Window, MouseBPressCallback);
			/// Vamos a crear la integracion del sistema de ventanas (WSI) para vulkan
			// EXT: VK_KHR_surface
			if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
				exit(-2);
#else
			const wchar_t CLASS_NAME[] = L"Vulkan renderer";
			WNDCLASS wc = {};
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = hInstance;
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.lpszClassName = "Vulkan renderer";
			RegisterClass(&wc);

			hwnd = CreateWindowEx(0, "Vulkan renderer", LPCSTR(L"Vulkan renderer"), WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, g_WindowWidth, g_WindowHeight,
				NULL, NULL, hInstance, NULL);
			ShowWindow(hwnd, SW_SHOW);
			VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.pNext = NULL;
			surfaceCreateInfo.flags = 0;
			surfaceCreateInfo.hinstance = hInstance;
			surfaceCreateInfo.hwnd = hwnd;
			auto result = vkCreateWin32SurfaceKHR(m_Instance, &surfaceCreateInfo, NULL, &m_Surface);
			if (result != VK_SUCCESS)
			{
				printf("Failed to create surface");
				exit(-1);
			}
#endif
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
			if (CreateDebugUtilsMessengerEXT(m_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
			{
				fprintf(stderr, "ERROR CREATING DEBUG MESSENGER\n");
			}
			g_context.CreateDevice(m_Instance);
			// Present support on the Physical Device
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(g_context.m_GpuInfo.m_Device, g_context.m_GraphicsComputeQueueFamilyIndex, m_Surface, &presentSupport);
			if (!presentSupport)
				printf("CANNOT PRESENT ON THIS DEVICE: %s\n", g_context.m_GpuInfo.m_Properties.deviceName);
			vkGetDeviceQueue(g_context.m_LogicDevice, g_context.m_GraphicsComputeQueueFamilyIndex, 0, &g_context.m_PresentQueue);

			/* Ahora tenemos que ver que nuestro device soporta la swapchain
			 * y sus capacidades.
			*/
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_context.m_GpuInfo.m_Device, m_Surface, &m_Capabilities);
			unsigned int surfaceFormatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(g_context.m_GpuInfo.m_Device, m_Surface, &surfaceFormatCount, nullptr);
			std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(g_context.m_GpuInfo.m_Device, m_Surface, &surfaceFormatCount, surfaceFormats.data());
			unsigned int presentModesCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(g_context.m_GpuInfo.m_Device, m_Surface, &presentModesCount, nullptr);
			std::vector<VkPresentModeKHR> presentModes(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(g_context.m_GpuInfo.m_Device, m_Surface, &presentModesCount, presentModes.data());
			/// Ahora elegimos el formato que cumpla nuestras necesidades.
			unsigned int formatChoosen = 0;
			for (const auto& format : surfaceFormats)
			{
				if (format.format == VK_FORMAT_B8G8R8A8_SRGB
					&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					m_SurfaceFormat = surfaceFormats[formatChoosen];
					break;
				}
				++formatChoosen;
			}
			/// Elegimos el modo de presentacion a la pantalla
			///	VK_PRESENT_MODE_IMMEDIATE_KHR
			/// VK_PRESENT_MODE_FIFO_KHR (el unico que nos aseguran que este disponible)
			/// VK_PRESENT_MODE_FIFO_RELAXED_KHR
			/// VK_PRESENT_MODE_MAILBOX_KHR
			m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
			for (const auto& presentMode : presentModes)
			{
				if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					m_PresentMode = presentMode;
					break;
				}
			}
			/// ahora vamos a establecer el swap extent, que es la resolucion de las imagnes de la swapchain
			if(m_Capabilities.currentExtent.width > m_Capabilities.maxImageExtent.width || m_Capabilities.currentExtent.height > m_Capabilities.maxImageExtent.height)
				m_CurrentExtent = VkExtent2D(g_WindowWidth, g_WindowHeight);
			else
				m_CurrentExtent = m_Capabilities.currentExtent;
			/// Ahora creamos la swapchain como tal.
			m_SwapchainImagesCount = m_Capabilities.minImageCount;
			VMA_Initialize(g_context.m_GpuInfo.m_Device, g_context.m_LogicDevice, m_Instance);
			CreateSwapChain();

			m_CubemapRender = new CubemapRenderer(g_context.m_LogicDevice);
			m_ShadowRender = new ShadowRenderer(g_context.m_LogicDevice);
			m_DbgRender = new DebugRenderer(g_context.m_LogicDevice);
			m_GridRender = new ShaderRenderer(g_context.m_LogicDevice);
			m_QuadRender = new QuadRenderer(g_context.m_LogicDevice);
			
			// Creamos los DescriptorsLayout
			// Inicializar Renderer
			// Setup de PipelineLayout
			// Creamos la PipelileLayout
			// Crear Renderpass
			// Crear Pipeline
			// Limpiar ShaderModules
			
			m_ShadowMat = new R_ShadowMaterial();
			m_ShadowMat->CreateDescriptorPool(g_context.m_LogicDevice);
			m_ShadowMat->CreateDescriptorSet(g_context.m_LogicDevice, m_ShadowRender->m_DescSetLayout);

			g_context.CreateRenderPass(&m_SwapChainCreateInfo);
			g_context.CreateGeometryPass(&m_SwapChainCreateInfo);

			m_DbgRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_DbgRender->CreatePipeline(g_context.m_RenderPass->pass);
			m_DbgRender->CleanShaderModules();

			m_ShadowRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			g_context.CreateShadowRenderPass();
			m_ShadowRender->CreatePipeline(g_context.m_ShadowPass->pass);
			m_ShadowRender->CleanShaderModules();

			m_CubemapRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_CubemapRender->CreatePipeline(g_context.m_RenderPass->pass);
			m_CubemapRender->CleanShaderModules();

			m_GridRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_GridRender->CreatePipeline(g_context.m_RenderPass->pass);
			m_GridRender->CleanShaderModules();

			// Quad Renderer
			m_QuadRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_QuadRender->CreatePipeline(g_context.m_RenderPass->pass);
			m_QuadRender->CleanShaderModules();

			vkGetPhysicalDeviceMemoryProperties(g_context.m_GpuInfo.m_Device, &m_Mem_Props);
			CreateCommandBuffer();
			// Creamos los recursos para el Shadow map
			CreateShadowResources();
			CreateShadowFramebuffer();
			// Creamos los recursos para el Depth testing
			CreateDepthTestingResources();
			CreateFramebuffers();
			//CreateGBufferImage();

			// Light buffers
			m_LightsBuffers.resize(FRAMES_IN_FLIGHT);
			m_LightsBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_LightsBuffersMapped.resize(FRAMES_IN_FLIGHT);

			m_QuadBuffers.resize(FRAMES_IN_FLIGHT);
			m_QuadBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_QuadBuffersMapped.resize(FRAMES_IN_FLIGHT);

			GenerateBuffers();
			GenerateDBGBuffers();
			CreateSyncObjects(0);
			CreateSyncObjects(1);
			CreatePerformanceQueries();
			m_GPipelineStatus = READY;

		}

		void VKBackend::GenerateDBGBuffers()
		{
#if 0
			// first clean old buffers
			VkDeviceSize dbgBufferSize = sizeof(DebugUniformBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				if(m_DbgRender->m_UniformBuffers[i])
				{
					vkDestroyBuffer(g_context.m_LogicDevice, m_DbgRender->m_UniformBuffers[i], nullptr);
					vkFreeMemory(g_context.m_LogicDevice, m_DbgRender->m_UniformBuffersMemory[i], nullptr);
				}
				utils::CreateBuffer(dbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgRender->m_UniformBuffers[i], m_DbgRender->m_UniformBuffersMemory[i]);
				vkMapMemory(g_context.m_LogicDevice, m_DbgRender->m_UniformBuffersMemory[i], 0,
					dbgBufferSize, 0, &m_DbgRender->m_UniformBuffersMapped[i]);
			}
			VkDeviceSize dynDbgBufferSize = m_CurrentDebugModelsToDraw * sizeof(DynamicBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				if(m_DbgRender->m_DynamicBuffers[i])
				{
					vkDestroyBuffer(g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffers[i], nullptr);
					vkFreeMemory(g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffersMemory[i], nullptr);
				}
				utils::CreateBuffer(dynDbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgRender->m_DynamicBuffers[i], m_DbgRender->m_DynamicBuffersMemory[i]);
				vkMapMemory(g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffersMemory[i], 0,
					dynDbgBufferSize, 0, &m_DbgRender->m_DynamicBuffersMapped[i]);
			}
#endif
		}
		void VKBackend::GenerateBuffer(size_t _sizeDynAl, VkBuffer* _buffers, 
			VkDeviceMemory* _buffsMemory, void** _mapped)
		{
#pragma region BUFFER
				auto DynAlign = _sizeDynAl;
				DynAlign = (DynAlign + g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
					& ~(g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
				VkDeviceSize checkBufferSize = (4 * DynAlign);
				VkDeviceSize bufferSize = (4 * _sizeDynAl);
				if (bufferSize != checkBufferSize)
#ifdef _MSVC
					__debugbreak();
#else
						raise(SIGTRAP);
#endif
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						_buffers[i], _buffsMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, _buffsMemory[i], 0,
						bufferSize, 0, &_mapped[i]);
				}
#pragma endregion
		}
		void VKBackend::GenerateBuffers()
		{
			GenerateBuffer(sizeof(LightBufferObject), m_LightsBuffers.data(),
				m_LightsBuffersMemory.data(), m_LightsBuffersMapped.data());
			GenerateBuffer(sizeof(DebugUniformBufferObject), m_QuadBuffers.data(),
				m_QuadBuffersMemory.data(), m_QuadBuffersMapped.data());
#if 0
			#pragma region LIGHTS_BUFFER
			{
				auto lightDynAlign = sizeof(LightBufferObject);
				lightDynAlign = (lightDynAlign + g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
					& ~(g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
				VkDeviceSize checkBufferSize = (4 * lightDynAlign);
				VkDeviceSize lightsBufferSize = (4 * sizeof(LightBufferObject));
				if(lightsBufferSize != checkBufferSize )
				#ifdef WIN32
					__debugbreak();
				#else
					raise(SIGTRAP);
				#endif
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					utils::CreateBuffer(lightsBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_LightsBuffers[i], m_LightsBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_LightsBuffersMemory[i], 0,
						lightsBufferSize, 0, &m_LightsBuffersMapped[i]);
				}
			}
			#pragma endregion
#endif
#if 0
			#pragma region CUBEMAP_BUFFERS
			{
				VkDeviceSize cubemapBufferSize = sizeof(CubemapUniformBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					utils::CreateBuffer(cubemapBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_CubemapRender->m_UniformBuffers[i], m_CubemapRender->m_UniformBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_CubemapRender->m_UniformBuffersMemory[i], 0,
						cubemapBufferSize, 0, &m_CubemapRender->m_UniformBuffersMapped[i]);
				}
				constexpr VkDeviceSize dynCubemapBufferSize = sizeof(DynamicBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					utils::CreateBuffer(dynCubemapBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_CubemapRender->m_DynamicBuffers[i], m_CubemapRender->m_DynamicBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_CubemapRender->m_DynamicBuffersMemory[i], 0,
						dynCubemapBufferSize, 0, &m_CubemapRender->m_DynamicBuffersMapped[i]);
				}
			}
			#pragma endregion
			#pragma region SHADOW_BUFFERS
			{
				// SHADOW UNIFORM BUFFERS
				m_ShadowRender->m_UniformBuffers.resize(FRAMES_IN_FLIGHT);
				m_ShadowRender->m_UniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
				m_ShadowRender->m_UniformBuffersMapped.resize(FRAMES_IN_FLIGHT);
				VkDeviceSize bufferSize = sizeof(ShadowUniformBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_ShadowRender->m_UniformBuffers[i], m_ShadowRender->m_UniformBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_ShadowRender->m_UniformBuffersMemory[i], 0,
						bufferSize, 0, &m_ShadowRender->m_UniformBuffersMapped[i]);
				}
				// Dynamic buffers
				m_ShadowRender->m_DynamicBuffers.resize(FRAMES_IN_FLIGHT);
				m_ShadowRender->m_DynamicBuffersMemory.resize(FRAMES_IN_FLIGHT);
				m_ShadowRender->m_DynamicBuffersMapped.resize(FRAMES_IN_FLIGHT);
				VkDeviceSize dynBufferSize = MAX_MODELS * sizeof(DynamicBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					utils::CreateBuffer(dynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_ShadowRender->m_DynamicBuffers[i],
						m_ShadowRender->m_DynamicBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_ShadowRender->m_DynamicBuffersMemory[i], 0,
						dynBufferSize, 0, &m_ShadowRender->m_DynamicBuffersMapped[i]);
				}
				// Shadow DescriptorSet
				m_ShadowMat->UpdateDescriptorSet(g_context.m_LogicDevice, m_ShadowRender->m_UniformBuffers, m_ShadowRender->m_DynamicBuffers);
			}
			#pragma endregion
			#pragma region GRID_BUFFERS
			//// GRID UNIFORM BUFFERS
			//m_GridUniformBuffers.resize(FRAMES_IN_FLIGHT);
			//m_GridUniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
			//m_GridUniformBuffersMapped.resize(FRAMES_IN_FLIGHT);
			//bufferSize = sizeof(DebugUniformBufferObject);
			//for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			//{
			//	utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			//		VK_SHARING_MODE_CONCURRENT,
			//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			//		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			//		m_GridUniformBuffers[i], m_GridUniformBuffersMemory[i]);
			//	vkMapMemory(g_context.m_LogicDevice, m_GridUniformBuffersMemory[i], 0,
			//		bufferSize, 0, &m_GridUniformBuffersMapped[i]);
			//}
			#pragma endregion
			#pragma region COMPUTE_BUFFERS
			{
				// COMPUTE BUFFERS
				m_ComputeUniformBuffers.resize(FRAMES_IN_FLIGHT);
				m_ComputeUniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
				m_ComputeUniformBuffersMapped.resize(FRAMES_IN_FLIGHT);
				VkDeviceSize bufferSize = sizeof(ComputeBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						VK_SHARING_MODE_CONCURRENT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
						m_ComputeUniformBuffers[i], m_ComputeUniformBuffersMemory[i]);

					vkMapMemory(g_context.m_LogicDevice, m_ComputeUniformBuffersMemory[i], 0,
						bufferSize, 0, &m_ComputeUniformBuffersMapped[i]);
				}
			}
			#pragma endregion
#endif
		}

		void VKBackend::InitializeVulkan(VkApplicationInfo* _appInfo)
		{
			_appInfo->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			_appInfo->pApplicationName = "Hello, Sailor";
			_appInfo->applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			_appInfo->pEngineName = "Raisin";
			_appInfo->engineVersion = VK_MAKE_VERSION(1, 0, 0);
			_appInfo->apiVersion = VK_API_VERSION_1_3;
		}

		void VKBackend::CreateInstance(VkInstanceCreateInfo* _createInfo, VkApplicationInfo* _appInfo, uint32_t m_extensionCount)
		{
			_createInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			_createInfo->pApplicationInfo = _appInfo;
			_createInfo->enabledExtensionCount = m_extensionCount;
			_createInfo->ppEnabledExtensionNames = m_InstanceExtensions.data();
			_createInfo->enabledLayerCount = (uint32_t)g_context.m_GpuInfo.m_ValidationLayers.size();
			_createInfo->ppEnabledLayerNames = g_context.m_GpuInfo.m_ValidationLayers.data();
		}

		void VKBackend::CreateSwapChain()
		{
			m_SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			m_SwapChainCreateInfo.surface = m_Surface;
			m_SwapChainCreateInfo.minImageCount = m_SwapchainImagesCount;
			m_SwapChainCreateInfo.imageFormat = m_SurfaceFormat.format;
			m_SwapChainCreateInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
			m_SwapChainCreateInfo.imageExtent = m_CurrentExtent;
			m_SwapChainCreateInfo.imageArrayLayers = 1;
			//VK_IMAGE_USAGE_TRANSFER_DST_BIT
			m_SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			m_SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//_swapChainCreateInfo->pQueueFamilyIndices = m_Gr;
			m_SwapChainCreateInfo.preTransform = m_Capabilities.currentTransform;
			m_SwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			m_SwapChainCreateInfo.presentMode = m_PresentMode;
			m_SwapChainCreateInfo.clipped = VK_TRUE;
			m_SwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
			auto swpResult = vkCreateSwapchainKHR(g_context.m_LogicDevice, &m_SwapChainCreateInfo, nullptr, &m_SwapChain);
			if (swpResult != VK_SUCCESS)
				exit(-3);
#if 0
			vkGetSwapchainImagesKHR(g_context.m_LogicDevice, m_SwapChain, &m_SwapchainImagesCount, nullptr);
			m_SwapChainImages.resize(m_SwapchainImagesCount);
#endif
			m_SwapchainImagesCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			VkImage tempswapchainimg[FRAMES_IN_FLIGHT];
			vkGetSwapchainImagesKHR(g_context.m_LogicDevice, m_SwapChain, &m_SwapchainImagesCount, tempswapchainimg);
			m_SwapchainImages[0] = NEW(Texture);
			m_SwapchainImages[0]->vk_image.image = tempswapchainimg[0];
			m_SwapchainImages[0]->vk_image.extent.height = m_Capabilities.currentExtent.height;
			m_SwapchainImages[0]->vk_image.extent.width  = m_Capabilities.currentExtent.width;
			m_SwapchainImages[0]->vk_image.extent.depth  = 1;
			m_SwapchainImages[0]->vk_image.format = VK_FORMAT_B8G8R8A8_SRGB;
			m_SwapchainImages[1] = NEW(Texture);
			m_SwapchainImages[1]->vk_image.image = tempswapchainimg[1];
			m_SwapchainImages[1]->vk_image.extent.height = m_Capabilities.currentExtent.height;
			m_SwapchainImages[1]->vk_image.extent.width = m_Capabilities.currentExtent.width;
			m_SwapchainImages[1]->vk_image.extent.depth = 1;
			m_SwapchainImages[1]->vk_image.format = VK_FORMAT_B8G8R8A8_SRGB;
			/// Ahora vamos a crear las vistas a la imagenes, para poder acceder a ellas y demas
			for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				CreateImageView(m_SwapchainImages[i], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
			}
		}
#if 1
		void VKBackend::RecreateSwapChain()
		{
			return; 
			// Si estamos minimizados, esperamos pacientemente a que se vuelva a ver la ventana
			int width = 0, height = 0;
#ifdef USE_GLFW
			glfwGetFramebufferSize(m_Window, &width, &height);
			while (width == 0 || height == 0)
			{
				glfwWaitEvents();
				glfwGetFramebufferSize(m_Window, &width, &height);
			}
#endif
			g_WindowHeight = height;
			g_WindowWidth = width;
			printf("\n-----------Re-create Swapchain %d x %d----------------\n", g_WindowWidth, g_WindowHeight);
			vkDeviceWaitIdle(g_context.m_LogicDevice);
			// Esperamos a que termine de pintarse y recreamos la swapchain con los nuevos parametros
			CleanSwapChain();
			CreateFramebufferAndSwapchain();
			m_NeedToRecreateSwapchain = false;
		}

		void VKBackend::CreateFramebufferAndSwapchain()
		{
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_context.m_GpuInfo.m_Device, m_Surface, &m_Capabilities);
			CreateSwapChain();
			g_context.CreateRenderPass(&m_SwapChainCreateInfo);
			m_ShadowRender->Initialize("engine/shaders/Shadow.vert", "engine/shaders/Shadow.frag", 1, &m_ShadowBindingDescription, m_ShadowAttributeDescriptions.size(), m_ShadowAttributeDescriptions.data());
			m_ShadowRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_ShadowRender->CreatePipelineLayout();
			g_context.CreateShadowRenderPass();
			m_ShadowRender->CreatePipeline(g_context.m_ShadowPass->pass);
			CreateShadowResources();
			CreateShadowFramebuffer();
			CreateDepthTestingResources();
			CreateFramebuffers();
		}
#endif

		void VKBackend::CreateFramebuffers()
		{
			// FRAMEBUFFERS
			m_SwapChainFramebuffers.resize(FRAMES_IN_FLIGHT);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				std::array<VkImageView, 2> attachments = {
					m_SwapchainImages[i]->vk_image.view,
					m_DepthTexture->vk_image.view
				};
				VkFramebufferCreateInfo mFramebufferInfo{};
				mFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				mFramebufferInfo.renderPass = g_context.m_RenderPass->pass;
				mFramebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				mFramebufferInfo.pAttachments = attachments.data();
				mFramebufferInfo.width = m_CurrentExtent.width;
				mFramebufferInfo.height = m_CurrentExtent.height;
				mFramebufferInfo.layers = 1;
				if (vkCreateFramebuffer(g_context.m_LogicDevice, &mFramebufferInfo, nullptr,
					&m_SwapChainFramebuffers[i]) != VK_SUCCESS)
					exit(-10);
			}
		}

		void VKBackend::CreateShadowFramebuffer()
		{
			VkFramebufferCreateInfo mFramebufferInfo{};
			mFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			mFramebufferInfo.renderPass = g_context.m_ShadowPass->pass;
			mFramebufferInfo.attachmentCount = 1;
			mFramebufferInfo.pAttachments = &m_ShadowTexture->vk_image.view;
			mFramebufferInfo.width = m_CurrentExtent.width;
			mFramebufferInfo.height = m_CurrentExtent.height;
			mFramebufferInfo.layers = 1;
			if (vkCreateFramebuffer(g_context.m_LogicDevice, &mFramebufferInfo, nullptr,
				&m_ShadowFramebuffer) != VK_SUCCESS)
				exit(-10);
		}

		void VKBackend::CreateCommandBuffer()
		{
			// COMMAND BUFFERS
			/// Command Pool
			///	m_GraphicsComputeQueueFamilyIndex
			VkCommandPoolCreateInfo m_PoolInfo{};
			m_PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			m_PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			m_PoolInfo.queueFamilyIndex = g_context.m_GraphicsComputeQueueFamilyIndex;
			if (vkCreateCommandPool(g_context.m_LogicDevice, &m_PoolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
				exit(-11);
			VkCommandBufferAllocateInfo mAllocInfo{};
			mAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			mAllocInfo.commandPool = m_CommandPool;
			mAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			mAllocInfo.commandBufferCount = 2;
			if (vkAllocateCommandBuffers(g_context.m_LogicDevice, &mAllocInfo, m_CommandBuffer) != VK_SUCCESS)
				exit(-12);
		}

		void VKBackend::CreateSyncObjects(unsigned int _frameIdx)
		{
			// Creamos los elementos de sincronizacion
			VkSemaphoreCreateInfo m_SemaphoreInfo{};
			m_SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkFenceCreateInfo mFenceInfo{};
			mFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			mFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (vkCreateSemaphore(g_context.m_LogicDevice, &m_SemaphoreInfo, nullptr, &m_ImageAvailable[_frameIdx]) != VK_SUCCESS
				|| vkCreateSemaphore(g_context.m_LogicDevice, &m_SemaphoreInfo, nullptr, &m_RenderFinish[_frameIdx]) != VK_SUCCESS
				|| vkCreateFence(g_context.m_LogicDevice, &mFenceInfo, nullptr, &m_InFlight[_frameIdx]) != VK_SUCCESS
				)
				exit(-666);
		}

		void VKBackend::CreatePerformanceQueries()
		{
			// VkQueryPoolCreateInfo
			if (g_context.m_GpuInfo.m_Properties.limits.timestampPeriod <= 0 && g_context.m_GpuInfo.m_Properties.limits.timestampComputeAndGraphics != VK_TRUE)
#ifdef _MSVC
	__debugbreak();
#else
				raise(SIGTRAP);
#endif
			g_Timestamps.resize(2);
			VkBool32 presentSupport = false;
			VkQueryPoolCreateInfo timestampQuery{};
			timestampQuery.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
			timestampQuery.queryType = VK_QUERY_TYPE_TIMESTAMP;
			timestampQuery.queryCount = g_Timestamps.size();
			
			if (vkCreateQueryPool(g_context.m_LogicDevice, &timestampQuery, nullptr, &m_PerformanceQuery[0]) != VK_SUCCESS)
				exit(-999);
			if (vkCreateQueryPool(g_context.m_LogicDevice, &timestampQuery, nullptr, &m_PerformanceQuery[1]) != VK_SUCCESS)
				exit(-999);

		}

		void VKBackend::CreateShadowResources()
		{
			VkFormat depthFormat = g_context.m_GpuInfo.FindDepthTestFormat();
			m_ShadowTexture = NEW(Texture);
			CreateImage(m_ShadowTexture, VkExtent3D(m_CurrentExtent.width, m_CurrentExtent.height, 1)
							, depthFormat, VK_IMAGE_TILING_OPTIMAL
							, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
							, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, 1);
			BindTextureMemory(m_ShadowTexture);
			TransitionImageLayout(m_ShadowTexture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
							, m_CommandPool, 1, &GetVKContext().m_GraphicsComputeQueue);
			CreateImageView(m_ShadowTexture, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
			CreateShadowTextureSampler(m_ShadowTexture);
		}

		void VKBackend::CreateDepthTestingResources()
		{
			VkFormat depthFormat = g_context.m_GpuInfo.FindDepthTestFormat();
			m_DepthTexture = NEW(Texture);
			CreateImage(m_DepthTexture, VkExtent3D(m_CurrentExtent.width, m_CurrentExtent.height, 1)
				, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, 1);
			BindTextureMemory(m_DepthTexture);
			TransitionImageLayout(m_DepthTexture, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_CommandPool, 1, &GetVKContext().m_GraphicsComputeQueue);
			CreateImageView(m_DepthTexture, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
			//m_DepthTexture->CreateTextureSampler()
		}

		void VKBackend::CollectGPUTimestamps(unsigned int _FrameToPresent)
		{
			vkDeviceWaitIdle(g_context.m_LogicDevice);
			vkGetQueryPoolResults(
					g_context.m_LogicDevice,
					 m_PerformanceQuery[_FrameToPresent],
					0,
				 2,
					g_Timestamps.size() * sizeof(uint64_t),
					g_Timestamps.data(),
					sizeof(uint64_t),
					VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
			g_TimestampValue = static_cast<float>(g_Timestamps[1] - g_Timestamps[0]) * g_context.m_GpuInfo.m_Properties.limits.timestampPeriod / 1000000.0f;

		}

		void VKBackend::CleanSwapChain()
		{
			vkDestroySwapchainKHR(g_context.m_LogicDevice, m_SwapChain, nullptr);
			for (auto& framebuffer : m_SwapChainFramebuffers)
				vkDestroyFramebuffer(g_context.m_LogicDevice, framebuffer, nullptr);
			for (auto& image : m_SwapchainImages)
				CleanTextureData(image, g_context.m_LogicDevice);
		}

		bool VKBackend::BackendShouldClose()
		{
#ifndef USE_GLFW
			reset_devices();
#endif
			return m_CloseEngine;
		}

		void VKBackend::PollEvents()
		{
#ifndef USE_GLFW
			MSG message;
			while (PeekMessage(&message, hwnd, 0, 0, PM_REMOVE))
			{
				if (message.message == WM_QUIT) {
					m_CloseEngine = true;
				}
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
				process_input();
#else
			glfwPollEvents();
#endif
		}
		double VKBackend::GetTime() 
		{
#ifndef USE_GLFW
			return 0.016;
#else
			return glfwGetTime(); 
#endif // !USE_GLFW
		}

		void VKBackend::Cleanup()
		{
			if(g_LoadDataThread)
				g_LoadDataThread->join();
			printf("Cleanup\n");
			//vmaDestroyAllocator(m_Allocator);
			g_context.Cleanup();
			#ifdef WIN32
			glslang::FinalizeProcess();
			#endif
			vkDestroyFramebuffer(g_context.m_LogicDevice, m_ShadowFramebuffer, nullptr);
			//delete m_GraphicsRender;
			delete m_DbgRender;
			delete m_ShadowRender;
			delete m_CubemapRender;
			delete m_GridRender;

			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				vkDestroyBuffer(g_context.m_LogicDevice, m_LightsBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_LightsBuffersMemory[i], nullptr);
#if 0
				vkDestroyBuffer(g_context.m_LogicDevice, m_DbgRender->m_UniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_DbgRender->m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_ShadowRender->m_UniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_ShadowRender->m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_ShadowRender->m_DynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_ShadowRender->m_DynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_CubemapRender->m_UniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_CubemapRender->m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_CubemapRender->m_DynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_CubemapRender->m_DynamicBuffersMemory[i], nullptr);
#endif
				/*vkDestroyBuffer(g_context.m_LogicDevice, m_GridUniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_GridUniformBuffersMemory[i], nullptr);*/
			}
			for (auto& tex : m_TexturesCache)
			{
				CleanTextureData(tex, g_context.m_LogicDevice);
			}
			CleanTextureData(m_DepthTexture, g_context.m_LogicDevice);
			CleanTextureData(m_ShadowTexture, g_context.m_LogicDevice);

			vkDestroySemaphore(g_context.m_LogicDevice, m_ImageAvailable[0], nullptr);
			vkDestroySemaphore(g_context.m_LogicDevice, m_ImageAvailable[1], nullptr);
			vkDestroySemaphore(g_context.m_LogicDevice, m_RenderFinish[0], nullptr);
			vkDestroySemaphore(g_context.m_LogicDevice, m_RenderFinish[1], nullptr);
			vkDestroyFence(g_context.m_LogicDevice, m_InFlight[0], nullptr);
			vkDestroyFence(g_context.m_LogicDevice, m_InFlight[1], nullptr);
			vkDestroyCommandPool(g_context.m_LogicDevice, m_CommandPool, nullptr);
			vkDestroyQueryPool(g_context.m_LogicDevice, m_PerformanceQuery[0], nullptr);
			vkDestroyQueryPool(g_context.m_LogicDevice, m_PerformanceQuery[1], nullptr);
			CleanSwapChain();
			// m_Scene->Cleanup();
			m_ShadowMat->Cleanup(g_context.m_LogicDevice);
#ifndef USE_GLFW
			//vkDestroySurfaceKHR(instance, surface, nullptr);
			DestroyWindow(hwnd);
#endif
		}

		Texture* VKBackend::FindTexture(const char* _path)
		{
			for (auto& tex : m_TexturesCache)
			{
				if (strcmp(_path, tex->m_Path) == 0)
					return tex;
			}
			auto tex = NEW(Texture);
			tex->init(_path);
			LoadTexture(tex);
			if(tex->m_Mipmaps > 0)
				CreateAndTransitionImage(tex, m_CommandPool);
			else
				CreateAndTransitionImageNoMipMaps(tex, m_CommandPool);
			m_TexturesCache.push_back(tex);
			return tex;
		}

		void VKBackend::Shutdown()
		{
			printf("Shutdown\n");
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
			vkDestroyDevice(g_context.m_LogicDevice, nullptr);
			if (m_DebugMessenger != nullptr)
				DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
			vkDestroyInstance(m_Instance, nullptr);
		}
		void VKBackend::VMA_Initialize(VkPhysicalDevice _gpu, VkDevice _LogicDevice, VkInstance _instance)
		{
			VmaAllocatorCreateInfo alloc_info{};
			alloc_info.physicalDevice = _gpu;
			alloc_info.device = _LogicDevice;
			alloc_info.instance = _instance;
			alloc_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
			vmaCreateAllocator(&alloc_info, &vma_allocator);
		}

		void VKBackend::VMA_CreateBuffer(size_t _size, VkMemoryPropertyFlags _memProperties, VmaAllocation* allocation_, VkBuffer* buffer_)
		{

		}

		void VKBackend::VMA_CreateImage(VkMemoryPropertyFlags _memProperties, VkImageCreateInfo* _ImageCreateInfo
			, VkImage* Image_, VmaAllocation* Allocation_)
		{
			VmaAllocationCreateInfo vma_image_info{};
			vma_image_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			vma_image_info.requiredFlags = _memProperties;
			vma_image_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			VkResult result = vmaCreateImage(vma_allocator, _ImageCreateInfo, &vma_image_info, Image_, Allocation_, nullptr);
			if (result != VK_SUCCESS)
#ifdef _MSVC
				__debugbreak();
#else
				raise(SIGTRAP);
#endif
		}

		void VKBackend::VMA_BindTextureMemory(VkImage _image, VmaAllocation _allocation)
		{
			vkBindImageMemory(g_context.m_LogicDevice, _image, _allocation->GetMemory(), 0);
		}

		void VKBackend::VMA_DestroyImage(VkImage _image, VmaAllocation _allocation)
		{
			vmaDestroyImage(vma_allocator, _image, _allocation);
		}
		// VULKAN SPECIFIC
		void VKBackend::CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode, VkMemoryPropertyFlags _memFlags, VkBuffer& buffer_,
				VkDeviceMemory& bufferMem_)
		{
			auto renderContext = GetVKContext();
			unsigned int queueFamilyIndices[] = { renderContext.m_GraphicsComputeQueueFamilyIndex, renderContext.m_TransferQueueFamilyIndex };
			VkBufferCreateInfo mBufferInfo{};
			mBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			mBufferInfo.size = _size;
			mBufferInfo.usage = _usage;
			mBufferInfo.sharingMode = _shareMode;
			mBufferInfo.pQueueFamilyIndices = queueFamilyIndices;
			mBufferInfo.queueFamilyIndexCount = 2;

			if (vkCreateBuffer(renderContext.m_LogicDevice, &mBufferInfo, nullptr, &buffer_) != VK_SUCCESS)
				exit(-6);
			VkMemoryRequirements mem_Requ;
			vkGetBufferMemoryRequirements(renderContext.m_LogicDevice, buffer_, &mem_Requ);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = mem_Requ.size;
			allocInfo.memoryTypeIndex = renderContext.m_GpuInfo.FindMemoryType(mem_Requ.memoryTypeBits, _memFlags);
			if (vkAllocateMemory(renderContext.m_LogicDevice, &allocInfo, nullptr, &bufferMem_) != VK_SUCCESS)
				exit(-8);

			vkBindBufferMemory(renderContext.m_LogicDevice, buffer_, bufferMem_, 0);
		}

		inline
			void CreateSSBOBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode, VkMemoryPropertyFlags _memFlags, VkBuffer& buffer_,
				VkDeviceMemory& bufferMem_)
		{
			auto renderContext = GetVKContext();
			unsigned int queueFamilyIndices[] = { renderContext.m_GraphicsComputeQueueFamilyIndex, renderContext.m_TransferQueueFamilyIndex };
			VkBufferCreateInfo mBufferInfo{};
			mBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			mBufferInfo.size = _size;
			mBufferInfo.usage = _usage;
			mBufferInfo.sharingMode = _shareMode;
			mBufferInfo.pQueueFamilyIndices = queueFamilyIndices;
			mBufferInfo.queueFamilyIndexCount = 2;

			if (vkCreateBuffer(renderContext.m_LogicDevice, &mBufferInfo, nullptr, &buffer_) != VK_SUCCESS)
				exit(-6);
			VkMemoryRequirements mem_Requ;
			vkGetBufferMemoryRequirements(renderContext.m_LogicDevice, buffer_, &mem_Requ);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = mem_Requ.size;
			allocInfo.memoryTypeIndex = renderContext.m_GpuInfo.FindMemoryType(mem_Requ.memoryTypeBits, _memFlags);
			if (vkAllocateMemory(renderContext.m_LogicDevice, &allocInfo, nullptr, &bufferMem_) != VK_SUCCESS)
				exit(-8);

			vkBindBufferMemory(renderContext.m_LogicDevice, buffer_, bufferMem_, 0);
		}

		
		VkCommandBuffer VKBackend::BeginSingleTimeCommandBuffer(VkCommandPool _CommandPool)
		{
			auto renderContext = GetVKContext();
			VkCommandBufferAllocateInfo mAllocInfo{};
			mAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			mAllocInfo.commandPool = _CommandPool;
			mAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			mAllocInfo.commandBufferCount = 1;
			VkCommandBuffer commandBuffer;
			if (vkAllocateCommandBuffers(renderContext.m_LogicDevice, &mAllocInfo, &commandBuffer) != VK_SUCCESS)
				exit(-12);
			VkCommandBufferBeginInfo mBeginInfo{};
			mBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			mBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			if (vkBeginCommandBuffer(commandBuffer, &mBeginInfo) != VK_SUCCESS)
				exit(-13);
			return commandBuffer;
		}
		void VKBackend::EndSingleTimeCommandBuffer(VkCommandBuffer _commandBuffer, VkCommandPool _CommandPool, VkQueue _queue)
		{
			auto renderContext = GetVKContext();
			VK_ASSERT(vkEndCommandBuffer(_commandBuffer));
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &_commandBuffer;

			vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(_queue);
			vkFreeCommandBuffers(renderContext.m_LogicDevice, _CommandPool, 1, &_commandBuffer);
		}

		void VKBackend::CopyBuffer(VkBuffer dst_, VkBuffer _src, VkDeviceSize _size, VkCommandPool _CommandPool, VkQueue _queue)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(_CommandPool);
			// Copiar desde el Stagging buffer al buffer
			VkBufferCopy copyRegion{};
			copyRegion.size = _size;
			vkCmdCopyBuffer(commandBuffer, _src, dst_, 1, &copyRegion);
			EndSingleTimeCommandBuffer(commandBuffer, _CommandPool, _queue);
		}

		// TEXTURES
		void VKBackend::LoadTexture(Texture* _texture)
		{
			PERF_INIT("LOAD_TEXTURE");
			stbi_uc* pixels = nullptr;
			stbi_set_flip_vertically_on_load(true);
			int tWidth, tHeight, tChannels;
			pixels = stbi_load(_texture->m_Path, &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
			_texture->vk_image.extent = VkExtent3D(tWidth, tHeight, 1);
			if (!pixels)
			{
#ifdef _MSVC
				__debugbreak();
#else
				raise(SIGTRAP);
#endif
				exit(-666);
			}
			PERF_END("LOAD_TEXTURE");
			_texture->m_Mipmaps = (uint8_t)std::log2(tWidth > tHeight ? tWidth : tHeight);
			size_t size = tWidth * tHeight * 4;
			fprintf(stderr, "\tLoading texture %s size: %llu\n", _texture->m_Path, size);
			// Buffer transfer of pixels
			CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_StagingBuffer, m_StaggingBufferMemory);
			void* data;
			vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, size, 0, &data);
			memcpy(data, pixels, size);
			vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
			stbi_image_free(pixels);
		}
		void VKBackend::LoadCubemapTexture(Texture* _texture)
		{
			LoadTexture(_texture);
			//stbi_uc* pixels;

			//pixels = stbi_load(m_Path, &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
			//if (!pixels)
			//{
			//	printf("\rMissing Texture %s\n", m_Path);
			//	#ifdef WIN32
			//		__debugbreak();
			//	#else
			//		raise(SIGTRAP);
			//	#endif
			//	exit(-666);
			//}
			//tHeight=tWidth;
			//auto size = (tWidth * tHeight * 4);
			//// Buffer transfer of pixels
			//utils::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
			//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			//	m_StagingBuffer, m_StaggingBufferMemory);
			//void* data;
			//vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, size, 0, &data);
			//memcpy(data, pixels, static_cast<size_t>(size));
			//vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
			//stbi_image_free(pixels);

		}

		void VKBackend::CreateAndTransitionImage(Texture* _texture, VkCommandPool _CommandPool
			, VkFormat _format, VkImageAspectFlags _aspectMask, VkImageViewType _viewType
			, uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			CreateImage(_texture, _texture->vk_image.extent, _format, VK_IMAGE_TILING_OPTIMAL
				, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _arrayLayers, _flags, _texture->m_Mipmaps);
			BindTextureMemory(_texture);

			TransitionImageLayout(_texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				, _CommandPool, _arrayLayers, &GetVKContext().m_GraphicsComputeQueue, _texture->m_Mipmaps);
			CopyBufferToImage(_texture, m_StagingBuffer, _texture->vk_image.extent, 0, _CommandPool
				, &GetVKContext().m_GraphicsComputeQueue, 0);
			GenerateMipmap(_texture, _CommandPool, _texture->m_Mipmaps);
			TransitionImageLayout(_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				, _CommandPool, _arrayLayers, &GetVKContext().m_GraphicsComputeQueue, _texture->m_Mipmaps);
			CreateImageView(_texture, _aspectMask, _viewType, _arrayLayers, _texture->m_Mipmaps);
			CreateTextureSampler(_texture, _texture->m_Mipmaps);
			vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
			vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
		}

		void VKBackend::CreateAndTransitionImageNoMipMaps(Texture* _texture, VkCommandPool _CommandPool, VkFormat _format, VkImageAspectFlags _aspectMask
			, VkImageViewType _viewType, uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			CreateImage(_texture, _texture->vk_image.extent, _format, VK_IMAGE_TILING_OPTIMAL
				, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _arrayLayers, _flags, 1);
			BindTextureMemory(_texture);
			TransitionImageLayout(_texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				, _CommandPool, _arrayLayers, &GetVKContext().m_GraphicsComputeQueue, 1);
			CopyBufferToImage(_texture, m_StagingBuffer, _texture->vk_image.extent, 0, _CommandPool
				, &GetVKContext().m_GraphicsComputeQueue, 0);
			TransitionImageLayout(_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				, _CommandPool, _arrayLayers, &GetVKContext().m_GraphicsComputeQueue, 1);
			CreateImageView(_texture, _aspectMask, _viewType, _arrayLayers, 1);
			CreateTextureSampler(_texture, 1);
			vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
			vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
		}

		void VKBackend::CreateAndTransitionImageCubemap(Texture* _texture, VkCommandPool _CommandPool, VkFormat _format, VkImageAspectFlags _aspectMask
			, VkImageViewType _viewType, uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			CreateAndTransitionImageNoMipMaps(_texture, _CommandPool, _format, _aspectMask
				, _viewType, _arrayLayers, _flags);
		}
		void VKBackend::CreateImage(Texture* _texture, VkExtent3D _extent, VkFormat _format, VkImageTiling _tiling
			, VkImageUsageFlagBits _usage, VkMemoryPropertyFlags _memProperties
			, uint32_t _arrayLayers, VkImageCreateFlags _flags, uint8_t _mipmapLvls)
		{
			auto renderContext = GetVKContext();
			VkImageCreateInfo tImageInfo{};
			tImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			tImageInfo.imageType = VK_IMAGE_TYPE_2D;
			tImageInfo.extent = _extent;
			tImageInfo.extent.depth = 1;
			tImageInfo.mipLevels = _mipmapLvls;
			tImageInfo.arrayLayers = _arrayLayers;
			tImageInfo.format = _format;
			tImageInfo.tiling = _tiling;
			tImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			tImageInfo.usage = _usage;
			tImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			tImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			tImageInfo.flags = _flags;

			_texture->vk_image.extent = _extent;
			_texture->vk_image.format = _format;
#ifndef USE_VMA
			VK_ASSERT(vkCreateImage(renderContext.m_LogicDevice, &tImageInfo, nullptr, & _texture->vk_image.image));
			VkMemoryRequirements memRequ;
			vkGetImageMemoryRequirements(renderContext.m_LogicDevice,  _texture->vk_image.image, &memRequ);
			VkMemoryAllocateInfo tAllocInfo{};
			tAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			tAllocInfo.allocationSize = memRequ.size;
			tAllocInfo.memoryTypeIndex = renderContext.m_GpuInfo.FindMemoryType(memRequ.memoryTypeBits, _memProperties);
			VK_ASSERT(vkAllocateMemory(renderContext.m_LogicDevice, &tAllocInfo, nullptr, & _texture->vk_image.memory));
#else
			VMA_CreateImage(_memProperties, &tImageInfo, &_texture->vk_image.image, &_texture->vk_image.memory);
#endif
		}

		void VKBackend::GenerateMipmap(Texture* _texture, VkCommandPool _CommandPool, uint8_t _mipLevels)
		{
			auto renderContext = GetVKContext();
			VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(_CommandPool);
			VkImageMemoryBarrier iBarrier{};
			iBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			iBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.image = _texture->vk_image.image;
			iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			iBarrier.subresourceRange.layerCount = 1;
			iBarrier.subresourceRange.levelCount = 1;
			// copy/blit transactions
			for (uint8_t i = 1; i < _mipLevels; i++)
			{
				// pre-copy transition
				iBarrier.subresourceRange.baseMipLevel = i - 1;
				iBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				iBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				iBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				iBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr,
					1, &iBarrier);
				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { (int32_t)_texture->vk_image.extent.width,  (int32_t)_texture->vk_image.extent.height, 1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;

				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { (int32_t)(_texture->vk_image.extent.width > 1 ? _texture->vk_image.extent.width / 2 : 1)
					, (int32_t)(_texture->vk_image.extent.height > 1 ? _texture->vk_image.extent.height / 2 : 1)
					, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;
				vkCmdBlitImage(commandBuffer, _texture->vk_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					_texture->vk_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
					VK_FILTER_LINEAR);
				_texture->vk_image.extent.width =  _texture->vk_image.extent.width > 1 ?  _texture->vk_image.extent.width / 2 : 1;
				_texture->vk_image.extent.height = _texture->vk_image.extent.height > 1 ? _texture->vk_image.extent.height / 2 : 1;
				if (_texture->vk_image.extent.width == 1 || _texture->vk_image.extent.height == 1)
					break;
			}
			// Post-copy transactions
			iBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			iBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			iBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			iBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr,
				1, &iBarrier);
			EndSingleTimeCommandBuffer(commandBuffer, _CommandPool, renderContext.m_GraphicsComputeQueue);
		}

		void VKBackend::TransitionImageLayout(Texture* _texture, VkImageLayout _old, VkImageLayout _new, VkCommandPool _CommandPool
			, uint32_t _layerCount, VkQueue* _queue, uint8_t _levelCount)
		{
			auto renderContext = GetVKContext();
			VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(_CommandPool);
			VkImageMemoryBarrier iBarrier{};
			iBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			iBarrier.oldLayout = _old;
			iBarrier.newLayout = _new;
			iBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.image = _texture->vk_image.image;
			if (_new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (renderContext.HasStencilComponent(_texture->vk_image.format))
				{
					iBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else
				iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			iBarrier.subresourceRange.baseMipLevel = 0;
			iBarrier.subresourceRange.levelCount = _levelCount;
			iBarrier.subresourceRange.baseArrayLayer = 0;
			iBarrier.subresourceRange.layerCount = _layerCount;
			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (_old == VK_IMAGE_LAYOUT_UNDEFINED && _new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				iBarrier.srcAccessMask = 0;
				iBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (_old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				iBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				iBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (_old == VK_IMAGE_LAYOUT_UNDEFINED && _new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				iBarrier.srcAccessMask = 0;
				iBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else
			{
				printf("unsupported layout transition!");
				exit(-96);
			}
			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &iBarrier);
			EndSingleTimeCommandBuffer(commandBuffer, _CommandPool, *_queue);
		}

		void VKBackend::CopyBufferToImage(Texture* _texture, VkBuffer _buffer, VkExtent3D _extent
			, VkDeviceSize _bufferOffset, VkCommandPool _CommandPool
			, VkQueue* _queue, uint32_t _layer)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(_CommandPool);
			VkBufferImageCopy region{};
			region.bufferOffset = _bufferOffset;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = _layer;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = _extent;
			vkCmdCopyBufferToImage(commandBuffer, _buffer, _texture->vk_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			EndSingleTimeCommandBuffer(commandBuffer, _CommandPool, *_queue);
		}

		void VKBackend::CreateImageView(Texture* _texture, VkImageAspectFlags _aspectMask, VkImageViewType _viewType
			, uint32_t _arrayLayers, uint32_t _levelCount)
		{
			auto renderContext = GetVKContext();
			VkImageView tImageView;
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = _texture->vk_image.image;
			viewInfo.viewType = _viewType;
			viewInfo.format = _texture->vk_image.format;
			viewInfo.subresourceRange.aspectMask = _aspectMask;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = _levelCount;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = _arrayLayers;
			VK_ASSERT(vkCreateImageView(renderContext.m_LogicDevice, &viewInfo, nullptr, &_texture->vk_image.view));
		}

		void VKBackend::CreateTextureImageView(Texture* _texture)
		{
			// VK_FORMAT_R8G8B8A8_SRGB
			CreateImageView(_texture, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
		}

		void VKBackend::BindTextureMemory(Texture* _texture)
		{
#ifndef USE_VMA
			vkBindImageMemory(g_context.m_LogicDevice,  _texture->vk_image.image,  _texture->vk_image.memory, 0);
#else
			VMA_BindTextureMemory(_texture->vk_image.image, _texture->vk_image.memory);
#endif
		}

		void VKBackend::CreateTextureSampler(Texture* _textue, float _Mipmaps, VkSamplerAddressMode _u, VkSamplerAddressMode _v, VkSamplerAddressMode _w)
		{
			auto renderContext = GetVKContext();
			VkPhysicalDeviceProperties deviceProp;
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = _u;
			samplerInfo.addressModeV = _v;
			samplerInfo.addressModeW = _w;
			samplerInfo.anisotropyEnable = VK_TRUE;
			vkGetPhysicalDeviceProperties(renderContext.m_GpuInfo.m_Device, &deviceProp);
			samplerInfo.maxAnisotropy = deviceProp.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = _Mipmaps;
			VK_ASSERT(vkCreateSampler(renderContext.m_LogicDevice, &samplerInfo, nullptr, &_textue->m_Sampler));
		}

		void VKBackend::CreateShadowTextureSampler(Texture* _shadowTexture)
		{
			auto renderContext = GetVKContext();
			VkPhysicalDeviceProperties deviceProp;
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.anisotropyEnable = VK_TRUE;

			vkGetPhysicalDeviceProperties(renderContext.m_GpuInfo.m_Device, &deviceProp);
			samplerInfo.maxAnisotropy = deviceProp.limits.maxSamplerAnisotropy;

			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			//samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.f;
			samplerInfo.minLod = 0.f;
			samplerInfo.maxLod = 1.f;
			VK_ASSERT(vkCreateSampler(renderContext.m_LogicDevice, &samplerInfo, nullptr, &_shadowTexture->m_Sampler));
		}

		void VKBackend::CleanTextureData(Texture* _texture, VkDevice _LogicDevice)
		{
			vkDestroySampler(_LogicDevice, _texture->m_Sampler, nullptr);
			_texture->m_Sampler = nullptr;
			vkDestroyImageView(_LogicDevice, _texture->vk_image.view, nullptr);
#ifndef USE_VMA
			vkDestroyImage(_LogicDevice,  _texture->vk_image.image, nullptr);
			vkFreeMemory(_LogicDevice,  _texture->vk_image.memory, nullptr);
#else
			VMA_DestroyImage(_texture->vk_image.image, _texture->vk_image.memory);
#endif
		}
    }
}
