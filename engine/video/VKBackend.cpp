#include "VKBackend.h"
#include "../filesystem/ResourceManager.h"
#include "glslang/Public/ShaderLang.h"
#include "../core/Materials/VKRTexture.h"

#include "VKRUtils.h"

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#ifndef WIN32
#include <signal.h>
#endif

namespace VKR
{
    namespace render
    {
		void FramebufferResizeCallback(GLFWwindow* _window, int _newW, int _newH)
		{
			m_NeedToRecreateSwapchain = true;
		}

		// INPUT CALLBACKS
		void MouseInputCallback(GLFWwindow* _window, double _xPos, double _yPos)
		{
			double x_offset = (_xPos - m_LastXPosition);
			double y_offset = (m_LastYPosition - _yPos);
			float senseo = 0.1f;
			m_LastXPosition = _xPos;
			m_LastYPosition = _yPos;
			if (m_MouseCaptured)
			{
				x_offset *= senseo;
				y_offset *= senseo;
				m_CameraYaw   += x_offset;
				m_CameraPitch += y_offset;
				// CONSTRAINTS
				if (m_CameraPitch > 89.f)  m_CameraPitch = 89.f;
				if (m_CameraPitch < -89.f) m_CameraPitch = -89.f;
				glm::vec3 camera_direction;
				camera_direction.x = static_cast<float>(cos(glm::radians(m_CameraYaw) * cos(glm::radians(m_CameraPitch))));
				camera_direction.y = static_cast<float>(sin(glm::radians(m_CameraPitch)));
				camera_direction.z = static_cast<float>(sin(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch)));
				m_CameraForward = glm::normalize(camera_direction);
				m_LastXPosition = _xPos;
				m_LastYPosition = _yPos;
			}
		}

		void MouseBPressCallback(GLFWwindow* _window, int _button, int _action, int _mods)
		{
			if (_button == GLFW_MOUSE_BUTTON_RIGHT && _action == GLFW_PRESS && !m_MouseCaptured)
			{
				m_MouseCaptured = true;
			}
			else
				m_MouseCaptured = false;
		}

		void KeyboardInputCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
		{
			auto state = glfwGetKey(_window, _key);
			if (_key == GLFW_KEY_W && state)
			{
				m_CameraPos += m_CameraSpeed * m_CameraForward;
			}
			if (_key == GLFW_KEY_A && state)
			{
				m_CameraPos += m_CameraSpeed * glm::normalize(glm::cross(
					m_CameraUp, m_CameraForward));
			}
			if (_key == GLFW_KEY_S && state)
			{
				m_CameraPos -= m_CameraSpeed * m_CameraForward;
			}
			if (_key == GLFW_KEY_D && state)
			{
				m_CameraPos -= m_CameraSpeed * glm::normalize(glm::cross(
					m_CameraUp, m_CameraForward));
			}

			if (_key == GLFW_KEY_R && _action == GLFW_PRESS)
			{
				m_CameraPos = glm::vec3(0.f);
			}

			if (_key == GLFW_KEY_E && _action == GLFW_PRESS) // up
			{
				m_CameraPos = glm::vec3(m_CameraPos.x, m_CameraPos.y + m_CameraSpeed, m_CameraPos.z);
			}

			//if (_key == GLFW_KEY_C && _action == GLFW_PRESS) // up
			//{
			//	RM::_AddRequest(ASSIMP_MODEL,"resources/models/Sponza/glTF/", "Sponza.gltf");
			//}
			//if (_key == GLFW_KEY_V && _action == GLFW_PRESS) // up
			//{
			//	//RM::_AddRequest(STATIC_MODEL,"resources/models/StainedGlassLamp/glTF/", "StainedGlassLamp.gltf");
			//	auto tempModel = new render::R_Model();
			//	auto data = filesystem::read_glTF("resources/models/StainedGlassLamp/glTF/", "StainedGlassLamp.gltf", tempModel);
			//	render::m_SceneDirty = true;
			//}

			//if (_key == GLFW_KEY_P && _action == GLFW_PRESS) // up
			//{
			//	RM::_AddRequest(STATIC_MODEL, "resources/models/Plane/glTF/", "Plane.gltf");
			//}

			if (_key == GLFW_KEY_Q && _action == GLFW_PRESS) // down
			{
				m_CameraPos = glm::vec3(m_CameraPos.x, m_CameraPos.y - m_CameraSpeed, m_CameraPos.z);
			}

			if (_key == GLFW_KEY_ESCAPE && state)
			{
				m_CloseEngine= true;
			}
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

		void VKBackend::Init()
		{
			#ifdef WIN32
			glslang::InitializeProcess();
			printf("glslang GLSL version: %s\n", glslang::GetGlslVersionString());
			#endif
			m_GPipelineStatus = CREATING;
			uint32_t mExtensionCount = 0;
			const char** mExtensions;
			/// VULKAN/glfw THINGS
			if (!g_context.m_GpuInfo.CheckValidationLayerSupport()) exit(-2);
			VkApplicationInfo mAppInfo = {};
			InitializeVulkan(&mAppInfo);
			// Fill Extension supported by GPU
			mExtensions = glfwGetRequiredInstanceExtensions(&mExtensionCount);
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
			CreateInstance(&m_InstanceCreateInfo, &mAppInfo, mExtensions, mExtensionCount);
			VkResult m_result = vkCreateInstance(&m_InstanceCreateInfo, nullptr, &m_Instance);
			if (m_result != VK_SUCCESS)
			{
				printf("ERROR CREATING INSTANCE VULKAN");
				exit(- 1);
			}
			/// NORMAL RENDER THINGS
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			m_Window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "Vulkan renderer", nullptr, nullptr);
			glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
			glfwSetKeyCallback(m_Window, KeyboardInputCallback);
			glfwSetCursorPosCallback(m_Window, MouseInputCallback);
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetMouseButtonCallback(m_Window, MouseBPressCallback);
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
			/// Vamos a crear la integracion del sistema de ventanas (WSI) para vulkan
			// EXT: VK_KHR_surface
			if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
				exit(-2);

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
			m_CurrentExtent = m_Capabilities.currentExtent;
			/// Ahora creamos la swapchain como tal.
			m_SwapchainImagesCount = m_Capabilities.minImageCount;
			VMA_Initialize(g_context.m_GpuInfo.m_Device, g_context.m_LogicDevice, m_Instance);
			CreateSwapChain();
			m_CubemapRender = new CubemapRenderer(g_context.m_LogicDevice);
			m_ShadowRender = new ShadowRenderer(g_context.m_LogicDevice);
			m_ShadowMat = new R_ShadowMaterial();
			m_DbgRender = new DebugRenderer(g_context.m_LogicDevice);
			m_GridRender = new ShaderRenderer(g_context.m_LogicDevice);
			// primero creamos el layout de los descriptors
			m_CubemapRender->CreateDescriptorSetLayout();

			m_ShadowRender->CreateDescriptorSetLayout();
			m_DbgRender->CreateDescriptorSetLayout();
			m_GridRender->CreateDescriptorSetLayout();
			// Shadow Descriptors
			m_ShadowMat->CreateDescriptorPool(g_context.m_LogicDevice);
			m_ShadowMat->CreateDescriptorSet(g_context.m_LogicDevice, m_ShadowRender->m_DescSetLayout);

			// Inicializar Renderer
			//m_GraphicsRender->Initialize();
			// Setup de la PipelineLayout
			//m_GraphicsRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			// CreamosLaPipelileLayout
			//m_GraphicsRender->CreatePipelineLayout();
			// Crear Renderpass
			g_context.CreateRenderPass(&m_SwapChainCreateInfo);
			g_context.CreateGeometryPass(&m_SwapChainCreateInfo);
			// Crear Pipeline
			//m_GraphicsRender->CreatePipeline(g_context.m_RenderPass->pass);
			// Limpiar ShaderModules
			//m_GraphicsRender->CleanShaderModules();

			// Lo mismo para el renderer de Debug
			m_DbgRender->Initialize();
			m_DbgRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_DbgRender->CreatePipelineLayout();
			m_DbgRender->CreatePipeline(g_context.m_RenderPass->pass);
			m_DbgRender->CleanShaderModules();

			// Lo Mismo para Shadows
			m_ShadowRender->Initialize();
				m_ShadowRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
				m_ShadowRender->CreatePipelineLayout();
			g_context.CreateShadowRenderPass();
			m_ShadowRender->CreatePipeline(g_context.m_ShadowPass->pass);
			m_ShadowRender->CleanShaderModules();

			// Pa'l cubemap
			// Lo mismo para el renderer de Debug
			m_CubemapRender->Initialize();
			m_CubemapRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_CubemapRender->CreatePipelineLayout();
			m_CubemapRender->CreatePipeline(g_context.m_RenderPass->pass);
			m_CubemapRender->CleanShaderModules();

			//Grid vertex Render
			m_GridRender->Initialize();
			m_GridRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_GridRender->CreatePipelineLayout();
			m_GridRender->CreatePipeline(g_context.m_RenderPass->pass);
			m_GridRender->CleanShaderModules();

			vkGetPhysicalDeviceMemoryProperties(g_context.m_GpuInfo.m_Device, &m_Mem_Props);
			CreateCommandBuffer();
			// Creamos los recursos para el Shadow map
			CreateShadowResources();
			CreateShadowFramebuffer();
			// Creamos los recursos para el Depth testing
			CreateDepthTestingResources();
			CreateFramebuffers();
			CreateGBufferImage();
			// Uniform buffers
			m_UniformBuffers.resize(FRAMES_IN_FLIGHT);
			m_UniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_Uniform_SBuffersMapped.resize(FRAMES_IN_FLIGHT);
			// Dynamic buffers
			m_DynamicBuffers.resize(FRAMES_IN_FLIGHT);
			m_DynamicBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_DynamicBuffersMapped.resize(FRAMES_IN_FLIGHT);
			// Light buffers
			m_LightsBuffers.resize(FRAMES_IN_FLIGHT);
			m_LightsBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_LightsBuffersMapped.resize(FRAMES_IN_FLIGHT);
			// Uniform buffers
			m_DbgUniformBuffers.resize(FRAMES_IN_FLIGHT);
			m_DbgUniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_DbgUniformBuffersMapped.resize(FRAMES_IN_FLIGHT);
			// Dynamic buffers
			m_DbgDynamicBuffers.resize(FRAMES_IN_FLIGHT);
			m_DbgDynamicBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_DbgDynamicBuffersMapped.resize(FRAMES_IN_FLIGHT);
			//
			// Uniform buffers
			m_CubemapUniformBuffers.resize(FRAMES_IN_FLIGHT);
			m_CubemapUniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_CubemapUniformBuffersMapped.resize(FRAMES_IN_FLIGHT);
			// Dynamic buffers
			m_CubemapDynamicBuffers.resize(FRAMES_IN_FLIGHT);
			m_CubemapDynamicBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_CubemapDynamicBuffersMapped.resize(FRAMES_IN_FLIGHT);

			// Grid buffers
			// Uniform buffers
			/*m_GridUniformBuffers.resize(FRAMES_IN_FLIGHT);
			m_GridUniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_GridUniformBuffersMapped.resize(FRAMES_IN_FLIGHT);*/

			GenerateBuffers();
			GenerateDBGBuffers();
			CreateSyncObjects(0);
			CreateSyncObjects(1);
			CreatePerformanceQueries();
			m_GPipelineStatus = READY;
		}

		void VKBackend::GenerateDBGBuffers()
		{
			// first clean old buffers
			VkDeviceSize dbgBufferSize = sizeof(DebugUniformBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				if(m_DbgUniformBuffers[i])
				{
					vkDestroyBuffer(g_context.m_LogicDevice, m_DbgUniformBuffers[i], nullptr);
					vkFreeMemory(g_context.m_LogicDevice, m_DbgUniformBuffersMemory[i], nullptr);
				}
				CreateBuffer(dbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgUniformBuffers[i], m_DbgUniformBuffersMemory[i]);
				vkMapMemory(g_context.m_LogicDevice, m_DbgUniformBuffersMemory[i], 0,
					dbgBufferSize, 0, &m_DbgUniformBuffersMapped[i]);
			}
			VkDeviceSize dynDbgBufferSize = m_CurrentDebugModelsToDraw * sizeof(DynamicBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				if(m_DbgDynamicBuffers[i])
				{
					vkDestroyBuffer(g_context.m_LogicDevice, m_DbgDynamicBuffers[i], nullptr);
					vkFreeMemory(g_context.m_LogicDevice, m_DbgDynamicBuffersMemory[i], nullptr);
				}
				CreateBuffer(dynDbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgDynamicBuffers[i], m_DbgDynamicBuffersMemory[i]);
				vkMapMemory(g_context.m_LogicDevice, m_DbgDynamicBuffersMemory[i], 0,
					dynDbgBufferSize, 0, &m_DbgDynamicBuffersMapped[i]);
			}
		}

		void VKBackend::GenerateBuffers()
		{
			#pragma region RENDER_BUFFERS
			{

				VkDeviceSize bufferSize = MAX_MODELS * sizeof(UniformBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_UniformBuffers[i], m_UniformBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_UniformBuffersMemory[i], 0,
						bufferSize, 0, &m_Uniform_SBuffersMapped[i]);
				}
				auto DynAlign =  sizeof(DynamicBufferObject);
				DynAlign = (DynAlign + g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
					& ~(g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
				VkDeviceSize checkBufferSize = MAX_MODELS *  DynAlign;
				VkDeviceSize dynBufferSize = MAX_MODELS *  sizeof(DynamicBufferObject);
				if(dynBufferSize != checkBufferSize )
				#ifdef WIN32
					__debugbreak();
				#else
					raise(SIGTRAP);
				#endif
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					CreateBuffer(dynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_DynamicBuffers[i], m_DynamicBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_DynamicBuffersMemory[i], 0,
						dynBufferSize, 0, &m_DynamicBuffersMapped[i]);
				}
			}
			#pragma endregion
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
					CreateBuffer(lightsBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_LightsBuffers[i], m_LightsBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_LightsBuffersMemory[i], 0,
						lightsBufferSize, 0, &m_LightsBuffersMapped[i]);
				}
			}
			#pragma endregion
			#pragma region CUBEMAP_BUFFERS
			{
				VkDeviceSize cubemapBufferSize = sizeof(CubemapUniformBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					CreateBuffer(cubemapBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_CubemapUniformBuffers[i], m_CubemapUniformBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_CubemapUniformBuffersMemory[i], 0,
						cubemapBufferSize, 0, &m_CubemapUniformBuffersMapped[i]);
				}
				constexpr VkDeviceSize dynCubemapBufferSize = sizeof(DynamicBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					CreateBuffer(dynCubemapBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_CubemapDynamicBuffers[i], m_CubemapDynamicBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_CubemapDynamicBuffersMemory[i], 0,
						dynCubemapBufferSize, 0, &m_CubemapDynamicBuffersMapped[i]);
				}
			}
			#pragma endregion
			#pragma region SHADOW_BUFFERS
			{
				// SHADOW UNIFORM BUFFERS
				m_ShadowUniformBuffers.resize(FRAMES_IN_FLIGHT);
				m_ShadowUniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
				m_ShadowUniformBuffersMapped.resize(FRAMES_IN_FLIGHT);
				VkDeviceSize bufferSize = sizeof(ShadowUniformBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_ShadowUniformBuffers[i], m_ShadowUniformBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_ShadowUniformBuffersMemory[i], 0,
						bufferSize, 0, &m_ShadowUniformBuffersMapped[i]);
				}
				// Dynamic buffers
				m_ShadowDynamicBuffers.resize(FRAMES_IN_FLIGHT);
				m_ShadowDynamicBuffersMemory.resize(FRAMES_IN_FLIGHT);
				m_ShadowDynamicBuffersMapped.resize(FRAMES_IN_FLIGHT);
				VkDeviceSize dynBufferSize = MAX_MODELS * sizeof(DynamicBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					CreateBuffer(dynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_ShadowDynamicBuffers[i], m_ShadowDynamicBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_ShadowDynamicBuffersMemory[i], 0,
						dynBufferSize, 0, &m_ShadowDynamicBuffersMapped[i]);
				}
				// Shadow DescriptorSet
				m_ShadowMat->UpdateDescriptorSet(g_context.m_LogicDevice, m_ShadowUniformBuffers, m_ShadowDynamicBuffers);
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
			//	CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
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
					CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 
							| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
							| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_ComputeUniformBuffers[i], m_ComputeUniformBuffersMemory[i]);
					vkMapMemory(g_context.m_LogicDevice, m_ComputeUniformBuffersMemory[i], 0,
						bufferSize, 0, &m_ComputeUniformBuffersMapped[i]);
				}
			}
			#pragma endregion
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

		void VKBackend::CreateInstance(VkInstanceCreateInfo* _createInfo, VkApplicationInfo* _appInfo, const char** m_Extensions,
			uint32_t m_extensionCount)
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
			m_SwapchainImages[0] = new Texture("");
			m_SwapchainImages[0]->vk_image.image = tempswapchainimg[0];
			m_SwapchainImages[0]->vk_image.extent.height = m_Capabilities.currentExtent.height;
			m_SwapchainImages[0]->vk_image.extent.width  = m_Capabilities.currentExtent.width;
			m_SwapchainImages[0]->vk_image.extent.depth  = 1;
			m_SwapchainImages[0]->vk_image.format = VK_FORMAT_B8G8R8A8_SRGB;
			m_SwapchainImages[1] = new Texture("");
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
#if 0
		void VKBackend::RecreateSwapChain()
		{
			// Si estamos minimizados, esperamos pacientemente a que se vuelva a ver la ventana
			int width = 0, height = 0;
			glfwGetFramebufferSize(m_Window, &width, &height);
			while (width == 0 || height == 0)
			{
				glfwWaitEvents();
				glfwGetFramebufferSize(m_Window, &width, &height);
			}
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
			m_ShadowRender->Initialize(true);
			m_ShadowRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_ShadowRender->CreatePipelineLayout();
			g_context.CreateShadowRenderPass();
			m_ShadowRender->CreatePipeline(g_context.m_ShadowPass->pass);
			CreateShadowResources();
			CreateShadowFramebuffer();
			CreateDepthTestingResources();
			CreateGBufferImage();
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
				#ifdef WIN32
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
			m_ShadowTexture = new Texture("");
			m_ShadowTexture->CreateImage(VkExtent3D(m_CurrentExtent.width, m_CurrentExtent.height, 1)
				, depthFormat, VK_IMAGE_TILING_OPTIMAL
				, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, 1);
			m_ShadowTexture->BindTextureMemory();
			m_ShadowTexture->TransitionImageLayout( VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				, m_CommandPool, 1, &GetVKContext().m_GraphicsComputeQueue);
			m_ShadowTexture->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
			m_ShadowTexture->CreateShadowTextureSampler();
		}

		void VKBackend::CreateDepthTestingResources()
		{
			VkFormat depthFormat = g_context.m_GpuInfo.FindDepthTestFormat();
			m_DepthTexture = new Texture("");
			m_DepthTexture->CreateImage( VkExtent3D(m_CurrentExtent.width, m_CurrentExtent.height, 1)
				, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, 1);
			m_DepthTexture->BindTextureMemory();
			m_DepthTexture->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_CommandPool, 1, &GetVKContext().m_GraphicsComputeQueue);
			m_DepthTexture->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
			//m_DepthTexture->CreateTextureSampler()
		}
		void VKBackend::CreateGBufferImage()
		{
			VkFormat GBufferFormat = VK_FORMAT_R32G32B32A32_UINT;
			m_GBufferTexture = new Texture("");
			m_GBufferTexture->CreateImage(VkExtent3D(m_CurrentExtent.width, m_CurrentExtent.height, 1)
				, GBufferFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, 1);
			m_GBufferTexture->BindTextureMemory();
			m_GBufferTexture->TransitionImageLayout( VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_CommandPool, 1, &GetVKContext().m_GraphicsComputeQueue);
			m_GBufferTexture->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
		}

		void VKBackend::BeginRenderPass(unsigned int _InFlightFrame)
		{
			std::array<VkClearValue, 2> clearValues;
			clearValues[0].color = defaultClearColor;
			clearValues[1].depthStencil = { 1.0f, 0 };
			// Render pass
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = g_context.m_RenderPass->pass;
			renderPassInfo.framebuffer = m_SwapChainFramebuffers[_InFlightFrame];
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = m_CurrentExtent;
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(m_CommandBuffer[_InFlightFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		}

		void VKBackend::EndRenderPass(unsigned int _InFlightFrame)
		{
			vkCmdEndRenderPass(m_CommandBuffer[_InFlightFrame]);
			if (vkEndCommandBuffer(m_CommandBuffer[_InFlightFrame]) != VK_SUCCESS)
				exit(-17);
		}

		uint32_t VKBackend::BeginFrame(unsigned int _InFlightFrame)
		{
			// Ahora vamos a simular el siguiente frame
			PollEvents();
			uint32_t imageIdx;
			vkWaitForFences(g_context.m_LogicDevice, 1, &m_InFlight[_InFlightFrame], VK_TRUE, UINT64_MAX);
			//vkGetQueryPoolResults(); frame anterior al que estamos simulando
			vkResetFences(g_context.m_LogicDevice, 1, &m_InFlight[_InFlightFrame]);
			vkResetCommandBuffer(m_CommandBuffer[_InFlightFrame], 0);
			auto acqResult = vkAcquireNextImageKHR(g_context.m_LogicDevice, m_SwapChain, UINT64_MAX, m_ImageAvailable[_InFlightFrame],
				VK_NULL_HANDLE, &imageIdx);
			if (acqResult == VK_ERROR_OUT_OF_DATE_KHR || acqResult == VK_SUBOPTIMAL_KHR)
			{
				//RecreateSwapChain();
				vkAcquireNextImageKHR(g_context.m_LogicDevice, m_SwapChain, UINT64_MAX, m_ImageAvailable[_InFlightFrame],VK_NULL_HANDLE, &imageIdx);
			}
			else if (acqResult != VK_SUCCESS && acqResult != VK_SUBOPTIMAL_KHR)
				exit(-69);
			// Record command buffer
			VkCommandBufferBeginInfo mBeginInfo{};
			mBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			mBeginInfo.flags = 0;
			mBeginInfo.pInheritanceInfo = nullptr;
			if (vkBeginCommandBuffer(m_CommandBuffer[_InFlightFrame], &mBeginInfo) != VK_SUCCESS)
				exit(-13);
			if(g_GPUTimestamp)
				vkCmdResetQueryPool(m_CommandBuffer[_InFlightFrame], m_PerformanceQuery[_InFlightFrame], 0, 2);
			m_LastImageIdx = imageIdx;
			return imageIdx;
		}

		void VKBackend::SubmitAndPresent(unsigned int _FrameToPresent, uint32_t* _imageIdx)
		{
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			VkSemaphore waitSemaphores[] = { m_ImageAvailable[_FrameToPresent] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_CommandBuffer[_FrameToPresent];
			VkSemaphore signalSemaphores[] = { m_RenderFinish[_FrameToPresent] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			auto submit = vkQueueSubmit(g_context.m_GraphicsComputeQueue, 1, &submitInfo, m_InFlight[_FrameToPresent]);
			if ( submit != VK_SUCCESS)
			{
				fprintf(stderr, "Error on the Submit");
				exit(-1);
			}
			// Presentacion: devolver el frame al swapchain para que salga por pantalla
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;
			VkSwapchainKHR swapChains[] = { m_SwapChain };
			if(m_SwapChain) // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR -> VK_IMAGE_LAYOUT_UNDEFINED
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = _imageIdx;
			presentInfo.pResults = nullptr;
			m_PresentResult = vkQueuePresentKHR(g_context.m_PresentQueue, &presentInfo);
			/*if ((m_PresentResult == VK_ERROR_OUT_OF_DATE_KHR || m_PresentResult == VK_SUBOPTIMAL_KHR)
				&& m_NeedToRecreateSwapchain)
				RecreateSwapChain();*/
			/*else if (m_PresentResult != VK_SUCCESS && m_PresentResult != VK_SUBOPTIMAL_KHR)
				exit(-69);*/
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
				image->CleanTextureData(g_context.m_LogicDevice);
		}

		bool VKBackend::BackendShouldClose()
		{
			return m_CloseEngine || glfwWindowShouldClose(m_Window);
		}

		void VKBackend::PollEvents()
		{
			glfwPollEvents();
		}
		double VKBackend::GetTime() 
		{
			return glfwGetTime(); 
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
				vkDestroyBuffer(g_context.m_LogicDevice, m_UniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_DynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_DynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_LightsBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_LightsBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_DbgUniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_DbgUniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_DbgDynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_DbgDynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_ShadowUniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_ShadowUniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_ShadowDynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_ShadowDynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_CubemapUniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_CubemapUniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_CubemapDynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_CubemapDynamicBuffersMemory[i], nullptr);

				/*vkDestroyBuffer(g_context.m_LogicDevice, m_GridUniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_GridUniformBuffersMemory[i], nullptr);*/
			}

			m_DepthTexture->CleanTextureData(g_context.m_LogicDevice);
			m_ShadowTexture->CleanTextureData(g_context.m_LogicDevice);
			m_GBufferTexture->CleanTextureData(g_context.m_LogicDevice);

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
		}


		void VKBackend::Shutdown()
		{
			printf("Shutdown\n");
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
			vkDestroyDevice(g_context.m_LogicDevice, nullptr);
			if (m_DebugMessenger != nullptr)
				DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
			vkDestroyInstance(m_Instance, nullptr);
			glfwDestroyWindow(m_Window);
			glfwTerminate();
		}
    }
}
