#include "VKBackend.h"
#include "../filesystem/ResourceManager.h"
#include "glslang/Public/ShaderLang.h"
#include "../core/Materials/VKRTexture.h"
#include "../memory/mem_alloc.h"
#include "VKRUtils.h"
#include "../input/DeviceInput.h"

#include <vulkan/vulkan.h>


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
			if (!utils::g_context.m_GpuInfo.CheckValidationLayerSupport()) exit(-2);
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
			utils::g_context.CreateDevice(m_Instance);
			// Present support on the Physical Device
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(utils::g_context.m_GpuInfo.m_Device, utils::g_context.m_GraphicsComputeQueueFamilyIndex, m_Surface, &presentSupport);
			if (!presentSupport)
				printf("CANNOT PRESENT ON THIS DEVICE: %s\n", utils::g_context.m_GpuInfo.m_Properties.deviceName);
			vkGetDeviceQueue(utils::g_context.m_LogicDevice, utils::g_context.m_GraphicsComputeQueueFamilyIndex, 0, &utils::g_context.m_PresentQueue);

			/* Ahora tenemos que ver que nuestro device soporta la swapchain
			 * y sus capacidades.
			*/
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(utils::g_context.m_GpuInfo.m_Device, m_Surface, &m_Capabilities);
			unsigned int surfaceFormatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(utils::g_context.m_GpuInfo.m_Device, m_Surface, &surfaceFormatCount, nullptr);
			std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(utils::g_context.m_GpuInfo.m_Device, m_Surface, &surfaceFormatCount, surfaceFormats.data());
			unsigned int presentModesCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(utils::g_context.m_GpuInfo.m_Device, m_Surface, &presentModesCount, nullptr);
			std::vector<VkPresentModeKHR> presentModes(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(utils::g_context.m_GpuInfo.m_Device, m_Surface, &presentModesCount, presentModes.data());
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
			m_CurrentExtent = m_Capabilities.currentExtent;
			/// Ahora creamos la swapchain como tal.
			m_SwapchainImagesCount = m_Capabilities.minImageCount;
			utils::VMA_Initialize(utils::g_context.m_GpuInfo.m_Device, utils::g_context.m_LogicDevice, m_Instance);
			CreateSwapChain();

			m_CubemapRender = new CubemapRenderer(utils::g_context.m_LogicDevice);
			m_ShadowRender = new ShadowRenderer(utils::g_context.m_LogicDevice);
			m_DbgRender = new DebugRenderer(utils::g_context.m_LogicDevice);
			m_GridRender = new ShaderRenderer(utils::g_context.m_LogicDevice);
			m_QuadRender = new QuadRenderer(utils::g_context.m_LogicDevice);
			
			// Creamos los DescriptorsLayout
			// Inicializar Renderer
			// Setup de PipelineLayout
			// Creamos la PipelileLayout
			// Crear Renderpass
			// Crear Pipeline
			// Limpiar ShaderModules
			
			m_ShadowMat = new R_ShadowMaterial();
			m_ShadowMat->CreateDescriptorPool(utils::g_context.m_LogicDevice);
			m_ShadowMat->CreateDescriptorSet(utils::g_context.m_LogicDevice, m_ShadowRender->m_DescSetLayout);

			utils::g_context.CreateRenderPass(&m_SwapChainCreateInfo);
			utils::g_context.CreateGeometryPass(&m_SwapChainCreateInfo);

			m_DbgRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_DbgRender->CreatePipeline(utils::g_context.m_RenderPass->pass);
			m_DbgRender->CleanShaderModules();

			m_ShadowRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			utils::g_context.CreateShadowRenderPass();
			m_ShadowRender->CreatePipeline(utils::g_context.m_ShadowPass->pass);
			m_ShadowRender->CleanShaderModules();

			m_CubemapRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_CubemapRender->CreatePipeline(utils::g_context.m_RenderPass->pass);
			m_CubemapRender->CleanShaderModules();

			m_GridRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_GridRender->CreatePipeline(utils::g_context.m_RenderPass->pass);
			m_GridRender->CleanShaderModules();

			// Quad Renderer
			m_QuadRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_QuadRender->CreatePipeline(utils::g_context.m_RenderPass->pass);
			m_QuadRender->CleanShaderModules();

			vkGetPhysicalDeviceMemoryProperties(utils::g_context.m_GpuInfo.m_Device, &m_Mem_Props);
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
					vkDestroyBuffer(utils::g_context.m_LogicDevice, m_DbgRender->m_UniformBuffers[i], nullptr);
					vkFreeMemory(utils::g_context.m_LogicDevice, m_DbgRender->m_UniformBuffersMemory[i], nullptr);
				}
				utils::CreateBuffer(dbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgRender->m_UniformBuffers[i], m_DbgRender->m_UniformBuffersMemory[i]);
				vkMapMemory(utils::g_context.m_LogicDevice, m_DbgRender->m_UniformBuffersMemory[i], 0,
					dbgBufferSize, 0, &m_DbgRender->m_UniformBuffersMapped[i]);
			}
			VkDeviceSize dynDbgBufferSize = m_CurrentDebugModelsToDraw * sizeof(DynamicBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				if(m_DbgRender->m_DynamicBuffers[i])
				{
					vkDestroyBuffer(utils::g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffers[i], nullptr);
					vkFreeMemory(utils::g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffersMemory[i], nullptr);
				}
				utils::CreateBuffer(dynDbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgRender->m_DynamicBuffers[i], m_DbgRender->m_DynamicBuffersMemory[i]);
				vkMapMemory(utils::g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffersMemory[i], 0,
					dynDbgBufferSize, 0, &m_DbgRender->m_DynamicBuffersMapped[i]);
			}
#endif
		}
		void VKBackend::GenerateBuffer(size_t _sizeDynAl, VkBuffer* _buffers, 
			VkDeviceMemory* _buffsMemory, void** _mapped)
		{
#pragma region BUFFER
				auto DynAlign = _sizeDynAl;
				DynAlign = (DynAlign + utils::g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
					& ~(utils::g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
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
					utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						_buffers[i], _buffsMemory[i]);
					vkMapMemory(utils::g_context.m_LogicDevice, _buffsMemory[i], 0,
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
				lightDynAlign = (lightDynAlign + utils::g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
					& ~(utils::g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
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
					vkMapMemory(utils::g_context.m_LogicDevice, m_LightsBuffersMemory[i], 0,
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
					vkMapMemory(utils::g_context.m_LogicDevice, m_CubemapRender->m_UniformBuffersMemory[i], 0,
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
					vkMapMemory(utils::g_context.m_LogicDevice, m_CubemapRender->m_DynamicBuffersMemory[i], 0,
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
					vkMapMemory(utils::g_context.m_LogicDevice, m_ShadowRender->m_UniformBuffersMemory[i], 0,
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
					vkMapMemory(utils::g_context.m_LogicDevice, m_ShadowRender->m_DynamicBuffersMemory[i], 0,
						dynBufferSize, 0, &m_ShadowRender->m_DynamicBuffersMapped[i]);
				}
				// Shadow DescriptorSet
				m_ShadowMat->UpdateDescriptorSet(utils::g_context.m_LogicDevice, m_ShadowRender->m_UniformBuffers, m_ShadowRender->m_DynamicBuffers);
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
			//	vkMapMemory(utils::g_context.m_LogicDevice, m_GridUniformBuffersMemory[i], 0,
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

					vkMapMemory(utils::g_context.m_LogicDevice, m_ComputeUniformBuffersMemory[i], 0,
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
			_createInfo->enabledLayerCount = (uint32_t)utils::g_context.m_GpuInfo.m_ValidationLayers.size();
			_createInfo->ppEnabledLayerNames = utils::g_context.m_GpuInfo.m_ValidationLayers.data();
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
			auto swpResult = vkCreateSwapchainKHR(utils::g_context.m_LogicDevice, &m_SwapChainCreateInfo, nullptr, &m_SwapChain);
			if (swpResult != VK_SUCCESS)
				exit(-3);
#if 0
			vkGetSwapchainImagesKHR(utils::g_context.m_LogicDevice, m_SwapChain, &m_SwapchainImagesCount, nullptr);
			m_SwapChainImages.resize(m_SwapchainImagesCount);
#endif
			m_SwapchainImagesCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			VkImage tempswapchainimg[FRAMES_IN_FLIGHT];
			vkGetSwapchainImagesKHR(utils::g_context.m_LogicDevice, m_SwapChain, &m_SwapchainImagesCount, tempswapchainimg);
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
				m_SwapchainImages[i]->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
			}
		}
#if 1
		void VKBackend::RecreateSwapChain()
		{
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
			vkDeviceWaitIdle(utils::g_context.m_LogicDevice);
			// Esperamos a que termine de pintarse y recreamos la swapchain con los nuevos parametros
			CleanSwapChain();
			CreateFramebufferAndSwapchain();
			m_NeedToRecreateSwapchain = false;
		}

		void VKBackend::CreateFramebufferAndSwapchain()
		{
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(utils::g_context.m_GpuInfo.m_Device, m_Surface, &m_Capabilities);
			CreateSwapChain();
			utils::g_context.CreateRenderPass(&m_SwapChainCreateInfo);
			m_ShadowRender->Initialize("engine/shaders/Shadow.vert", "engine/shaders/Shadow.frag", 1, &m_ShadowBindingDescription, m_ShadowAttributeDescriptions.size(), m_ShadowAttributeDescriptions.data());
			m_ShadowRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_ShadowRender->CreatePipelineLayout();
			utils::g_context.CreateShadowRenderPass();
			m_ShadowRender->CreatePipeline(utils::g_context.m_ShadowPass->pass);
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
				mFramebufferInfo.renderPass = utils::g_context.m_RenderPass->pass;
				mFramebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				mFramebufferInfo.pAttachments = attachments.data();
				mFramebufferInfo.width = m_CurrentExtent.width;
				mFramebufferInfo.height = m_CurrentExtent.height;
				mFramebufferInfo.layers = 1;
				if (vkCreateFramebuffer(utils::g_context.m_LogicDevice, &mFramebufferInfo, nullptr,
					&m_SwapChainFramebuffers[i]) != VK_SUCCESS)
					exit(-10);
			}
		}

		void VKBackend::CreateShadowFramebuffer()
		{
			VkFramebufferCreateInfo mFramebufferInfo{};
			mFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			mFramebufferInfo.renderPass = utils::g_context.m_ShadowPass->pass;
			mFramebufferInfo.attachmentCount = 1;
			mFramebufferInfo.pAttachments = &m_ShadowTexture->vk_image.view;
			mFramebufferInfo.width = m_CurrentExtent.width;
			mFramebufferInfo.height = m_CurrentExtent.height;
			mFramebufferInfo.layers = 1;
			if (vkCreateFramebuffer(utils::g_context.m_LogicDevice, &mFramebufferInfo, nullptr,
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
			m_PoolInfo.queueFamilyIndex = utils::g_context.m_GraphicsComputeQueueFamilyIndex;
			if (vkCreateCommandPool(utils::g_context.m_LogicDevice, &m_PoolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
				exit(-11);
			VkCommandBufferAllocateInfo mAllocInfo{};
			mAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			mAllocInfo.commandPool = m_CommandPool;
			mAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			mAllocInfo.commandBufferCount = 2;
			if (vkAllocateCommandBuffers(utils::g_context.m_LogicDevice, &mAllocInfo, m_CommandBuffer) != VK_SUCCESS)
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
			if (vkCreateSemaphore(utils::g_context.m_LogicDevice, &m_SemaphoreInfo, nullptr, &m_ImageAvailable[_frameIdx]) != VK_SUCCESS
				|| vkCreateSemaphore(utils::g_context.m_LogicDevice, &m_SemaphoreInfo, nullptr, &m_RenderFinish[_frameIdx]) != VK_SUCCESS
				|| vkCreateFence(utils::g_context.m_LogicDevice, &mFenceInfo, nullptr, &m_InFlight[_frameIdx]) != VK_SUCCESS
				)
				exit(-666);
		}

		void VKBackend::CreatePerformanceQueries()
		{
			// VkQueryPoolCreateInfo
			if (utils::g_context.m_GpuInfo.m_Properties.limits.timestampPeriod <= 0 && utils::g_context.m_GpuInfo.m_Properties.limits.timestampComputeAndGraphics != VK_TRUE)
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
			
			if (vkCreateQueryPool(utils::g_context.m_LogicDevice, &timestampQuery, nullptr, &m_PerformanceQuery[0]) != VK_SUCCESS)
				exit(-999);
			if (vkCreateQueryPool(utils::g_context.m_LogicDevice, &timestampQuery, nullptr, &m_PerformanceQuery[1]) != VK_SUCCESS)
				exit(-999);

		}

		void VKBackend::CreateShadowResources()
		{
			VkFormat depthFormat = utils::g_context.m_GpuInfo.FindDepthTestFormat();
			m_ShadowTexture = NEW(Texture);
			m_ShadowTexture->CreateImage(VkExtent3D(m_CurrentExtent.width, m_CurrentExtent.height, 1)
				, depthFormat, VK_IMAGE_TILING_OPTIMAL
				, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, 1);
			m_ShadowTexture->BindTextureMemory();
			m_ShadowTexture->TransitionImageLayout( VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				, m_CommandPool, 1, &utils::GetVKContext().m_GraphicsComputeQueue);
			m_ShadowTexture->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
			m_ShadowTexture->CreateShadowTextureSampler();
		}

		void VKBackend::CreateDepthTestingResources()
		{
			VkFormat depthFormat = utils::g_context.m_GpuInfo.FindDepthTestFormat();
			m_DepthTexture = NEW(Texture);
			m_DepthTexture->CreateImage( VkExtent3D(m_CurrentExtent.width, m_CurrentExtent.height, 1)
				, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, 1);
			m_DepthTexture->BindTextureMemory();
			m_DepthTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_CommandPool, 1, &utils::GetVKContext().m_GraphicsComputeQueue);
			m_DepthTexture->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
			//m_DepthTexture->CreateTextureSampler()
		}

		void VKBackend::CollectGPUTimestamps(unsigned int _FrameToPresent)
		{
			vkDeviceWaitIdle(utils::g_context.m_LogicDevice);
			vkGetQueryPoolResults(
					utils::g_context.m_LogicDevice,
					 m_PerformanceQuery[_FrameToPresent],
					0,
				 2,
					g_Timestamps.size() * sizeof(uint64_t),
					g_Timestamps.data(),
					sizeof(uint64_t),
					VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
			g_TimestampValue = static_cast<float>(g_Timestamps[1] - g_Timestamps[0]) * utils::g_context.m_GpuInfo.m_Properties.limits.timestampPeriod / 1000000.0f;

		}

		void VKBackend::CleanSwapChain()
		{
			vkDestroySwapchainKHR(utils::g_context.m_LogicDevice, m_SwapChain, nullptr);
			for (auto& framebuffer : m_SwapChainFramebuffers)
				vkDestroyFramebuffer(utils::g_context.m_LogicDevice, framebuffer, nullptr);
			for (auto& image : m_SwapchainImages)
				image->CleanTextureData(utils::g_context.m_LogicDevice);
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
			utils::g_context.Cleanup();
			#ifdef WIN32
			glslang::FinalizeProcess();
			#endif
			vkDestroyFramebuffer(utils::g_context.m_LogicDevice, m_ShadowFramebuffer, nullptr);
			//delete m_GraphicsRender;
			delete m_DbgRender;
			delete m_ShadowRender;
			delete m_CubemapRender;
			delete m_GridRender;

			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_LightsBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_LightsBuffersMemory[i], nullptr);
#if 0
				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_DbgRender->m_UniformBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_DbgRender->m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_DbgRender->m_DynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_ShadowRender->m_UniformBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_ShadowRender->m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_ShadowRender->m_DynamicBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_ShadowRender->m_DynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_CubemapRender->m_UniformBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_CubemapRender->m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_CubemapRender->m_DynamicBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_CubemapRender->m_DynamicBuffersMemory[i], nullptr);
#endif
				/*vkDestroyBuffer(utils::g_context.m_LogicDevice, m_GridUniformBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_GridUniformBuffersMemory[i], nullptr);*/
			}
			for (auto& tex : m_TexturesCache)
			{
				tex->CleanTextureData(utils::g_context.m_LogicDevice);
			}
			m_DepthTexture->CleanTextureData(utils::g_context.m_LogicDevice);
			m_ShadowTexture->CleanTextureData(utils::g_context.m_LogicDevice);

			vkDestroySemaphore(utils::g_context.m_LogicDevice, m_ImageAvailable[0], nullptr);
			vkDestroySemaphore(utils::g_context.m_LogicDevice, m_ImageAvailable[1], nullptr);
			vkDestroySemaphore(utils::g_context.m_LogicDevice, m_RenderFinish[0], nullptr);
			vkDestroySemaphore(utils::g_context.m_LogicDevice, m_RenderFinish[1], nullptr);
			vkDestroyFence(utils::g_context.m_LogicDevice, m_InFlight[0], nullptr);
			vkDestroyFence(utils::g_context.m_LogicDevice, m_InFlight[1], nullptr);
			vkDestroyCommandPool(utils::g_context.m_LogicDevice, m_CommandPool, nullptr);
			vkDestroyQueryPool(utils::g_context.m_LogicDevice, m_PerformanceQuery[0], nullptr);
			vkDestroyQueryPool(utils::g_context.m_LogicDevice, m_PerformanceQuery[1], nullptr);
			CleanSwapChain();
			// m_Scene->Cleanup();
			m_ShadowMat->Cleanup(utils::g_context.m_LogicDevice);
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
			tex->LoadTexture();
			if(tex->m_Mipmaps > 0)
				tex->CreateAndTransitionImage(m_CommandPool);
			else
				tex->CreateAndTransitionImageNoMipMaps(m_CommandPool);
			m_TexturesCache.push_back(tex);
			return tex;
		}

		void VKBackend::Shutdown()
		{
			printf("Shutdown\n");
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
			vkDestroyDevice(utils::g_context.m_LogicDevice, nullptr);
			if (m_DebugMessenger != nullptr)
				DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
			vkDestroyInstance(m_Instance, nullptr);
		}
    }
}
