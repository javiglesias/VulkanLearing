#include "VKBackend.h"

#include <vulkan/vulkan.h>
#include <sys/types.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../dependencies/stb/stb_image.h"


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
			float x_offset = (_xPos - m_LastXPosition);
			float y_offset = (m_LastYPosition - _yPos);
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
				if (m_CameraPitch > 89.0f)  m_CameraPitch = 89.0f;
				if (m_CameraPitch < -89.0f) m_CameraPitch = -89.0f;
				glm::vec3 camera_direction;
				camera_direction.x = cos(glm::radians(m_CameraYaw) * cos(glm::radians(m_CameraPitch)));
				camera_direction.y = sin(glm::radians(m_CameraPitch));
				camera_direction.z = sin(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
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

			if (_key == GLFW_KEY_ESCAPE && state)
			{
				m_CloseEngine= true;
			}
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
				fprintf(stderr, "\tMessage PERFORMANCE: %s\n", pCallbackData->pMessage);
				g_ConsoleMSG += "\tMessage PERFORMANCE:";
				g_ConsoleMSG += pCallbackData->pMessage;
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
				fprintf(stderr, "\n\tVLayer message: %s\n", pCallbackData->pMessage);
				break;
			default:
				break;
			}
			return VK_FALSE;
		}

        static VKContext g_context;
		static VKBackend g_backend;
		VKContext& GetVKContext() 
		{
			return g_context;
		}
		VKBackend& GetVKBackend()
		{
			return g_backend;
		}

		VkFormat GPUInfo::FindSupportedFormat(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features)
		{
			for (VkFormat format : _candidates)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(m_Device, format, &props);
				if (_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features)
					return format;
				if (_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & _features) == _features)
					return format;
			}
			g_ConsoleMSG += "No found supported format\n";
			printf("No found supported format\n");
			exit(-79);
		}
		VkFormat GPUInfo::FindDepthTestFormat()
		{
			auto candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
			auto tiling = VK_IMAGE_TILING_OPTIMAL;
			auto features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
			return FindSupportedFormat(candidates, tiling, features);
		}
		bool GPUInfo::CheckValidationLayerSupport()
		{
			unsigned int layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
			for (const char* layerName : m_ValidationLayers)
			{
				bool layerFound = false;
				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						return true;
					}
				}
			}

			return false;
		}
		unsigned int GPUInfo::FindMemoryType(unsigned int typeFilter, VkMemoryPropertyFlags properties)
		{
			for (unsigned int i = 0; i < m_MemProps.memoryTypeCount; i++)
			{
				if (typeFilter & (1 << i) && (m_MemProps.memoryTypes[i].propertyFlags & properties) == properties)
					return i;
			}
			printf("Not Found Memory Type %d", properties);
			exit(-9);
		}

		void GPUInfo::CheckValidDevice(VkInstance _Instance)
		{
			/// Asociamos el VkDevice a la GPU
			unsigned int deviceCount = 0;
			vkEnumeratePhysicalDevices(_Instance, &deviceCount, nullptr);
			if (deviceCount == 0)
			{
				fprintf(stderr, "Not GPU FOUND\n");
				exit(-1);
			}
			// Comprobamos que los Physical device cumplan lo necesario
			std::vector<VkPhysicalDevice> m_PhysicalDevices(deviceCount);
			vkEnumeratePhysicalDevices(_Instance, &deviceCount, m_PhysicalDevices.data());
			VkPhysicalDeviceProperties* deviceProp = new VkPhysicalDeviceProperties[deviceCount];
			VkPhysicalDeviceFeatures* deviceFeatures = new VkPhysicalDeviceFeatures[deviceCount];
			unsigned int m_GPUSelected = 0;
			for (int it = 0; it < deviceCount; it++)
			{
				vkGetPhysicalDeviceProperties(m_PhysicalDevices[it], &deviceProp[it]);
				if (strstr(deviceProp[it].deviceName, "NVIDIA") || strstr(deviceProp[it].deviceName, "AMD"))
				{
					m_GPUSelected = it;
					char tmp[512];
					sprintf(tmp, "\nGPU %d: %s\n", it, deviceProp[m_GPUSelected].deviceName);
					g_ConsoleMSG += tmp;
					g_context.m_GpuInfo.minUniformBufferOffsetAlignment = deviceProp[m_GPUSelected].limits.minUniformBufferOffsetAlignment;
				}
				vkGetPhysicalDeviceFeatures(m_PhysicalDevices[it], &deviceFeatures[it]);
			}
			m_Device = m_PhysicalDevices[m_GPUSelected];
			vkGetPhysicalDeviceProperties(m_Device, &m_Properties);
			vkGetPhysicalDeviceFeatures(m_Device, &m_Features);
			vkGetPhysicalDeviceMemoryProperties(m_Device, &m_MemProps);
		}

		// VK CONTEXT
		void VKContext::CreateLogicalDevice()
		{
			VkDeviceCreateInfo m_CreateInfo{};
			m_CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			m_CreateInfo.pQueueCreateInfos = g_context.m_GpuInfo.m_QueueCreateInfos.data();
			m_CreateInfo.queueCreateInfoCount = g_context.m_GpuInfo.m_QueueCreateInfos.size();
			m_CreateInfo.pEnabledFeatures = &m_GpuInfo.m_Features;
			m_CreateInfo.enabledExtensionCount = g_context.m_GpuInfo.m_DeviceExtensions.size();
			m_CreateInfo.ppEnabledExtensionNames = g_context.m_GpuInfo.m_DeviceExtensions.data();
			m_CreateInfo.enabledLayerCount = 0;

			if (vkCreateDevice(m_GpuInfo.m_Device, &m_CreateInfo, nullptr, &g_context.m_LogicDevice) !=
				VK_SUCCESS)
			{
				printf("Failed to create Logical Device");
				exit(-1);
			}
		}
		void VKContext::CreateDevice(VkInstance _Instance)
		{
			m_GpuInfo.CheckValidDevice(_Instance);
			/// Comprobamos que tengamos la extension VK_KHR_swapchain soportada en el device GPU
			unsigned int deviceExtensionCount;
			vkEnumerateDeviceExtensionProperties(m_GpuInfo.m_Device, nullptr, &deviceExtensionCount, nullptr);
			std::vector<VkExtensionProperties> deviceAvailableExtensions(deviceExtensionCount);
			vkEnumerateDeviceExtensionProperties(m_GpuInfo.m_Device, nullptr, &deviceExtensionCount, deviceAvailableExtensions.data());
			for (const auto& ext : deviceAvailableExtensions)
			{
				// VK_KHR_buffer_device_address and VK_EXT_buffer_device_address not at the same time
				if (strcmp(ext.extensionName, "VK_EXT_buffer_device_address") == 0)
					continue;
				g_context.m_GpuInfo.m_DeviceExtensions.push_back(ext.extensionName);
			}
			/// Buscamos las familias de Colas.
			unsigned int queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(m_GpuInfo.m_Device, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(m_GpuInfo.m_Device, &queueFamilyCount, queueFamilies.data());
			// VK_QUEUE_GRAPHICS_BIT
			bool searchingGraphics = true, searchingTransfer = true;
			for (const auto& queueFamily : queueFamilies)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					char tmp[256];
					sprintf(tmp, "Graphics Family: %d\n", m_TransferQueueFamilyIndex);
					g_ConsoleMSG += tmp;
					searchingGraphics = false;
				}
				if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					char tmp[256];
					sprintf(tmp, "Transfer Family: %d\n", m_TransferQueueFamilyIndex);
					g_ConsoleMSG += tmp;
					searchingTransfer = false;
				}

				m_GraphicsQueueFamilyIndex += searchingGraphics;
				m_TransferQueueFamilyIndex += searchingTransfer;
				if (!searchingGraphics && !searchingTransfer)
					break;
			}

			/// Ahora vamos a crear el device logico para interactuar con él
			float queuePriority = 1.f;

			g_context.m_GpuInfo.m_QueueCreateInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = m_GraphicsQueueFamilyIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
				});

			g_context.m_GpuInfo.m_QueueCreateInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = m_TransferQueueFamilyIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
				});

			// Create logical device associated to physical device
			CreateLogicalDevice();
			vkGetDeviceQueue(m_LogicDevice, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_LogicDevice, m_TransferQueueFamilyIndex, 0, &m_TransferQueue);
		}

		void VKContext::CreateRenderPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo)
		{
			// RENDER PASES
			/// Attachment description
			// Color
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = m_SwapChainCreateInfo->imageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			// Depth
			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = m_GpuInfo.FindDepthTestFormat(); // d32_SFLOAT
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// SUB-PASSES
			/// Attachment References
			// color
			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// depth
			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			/// Sub-pass
			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			/// Subpass dependencies
			VkSubpassDependency subpassDep{};
			subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDep.dstSubpass = 0;
			subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			subpassDep.srcAccessMask = 0;
			subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			/// Render pass
			std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &subpassDep;
			if (vkCreateRenderPass(m_LogicDevice, &renderPassInfo, nullptr,
				&m_RenderPass) != VK_SUCCESS)
				exit(-8);

			// m_RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			// m_RenderPassInfo.attachmentCount = 1;
			// m_RenderPassInfo.pAttachments = &m_ColorAttachment;
			// m_RenderPassInfo.subpassCount = 1;
			// m_RenderPassInfo.pSubpasses = &m_Subpass;
			// m_RenderPassInfo.dependencyCount = 1;
			// m_RenderPassInfo.pDependencies = &m_SubpassDep;
			// if (vkCreateRenderPass(m_LogicDevice, &m_RenderPassInfo, nullptr,
			// 	&m_UIRenderPass) != VK_SUCCESS)
			// 	exit(-8);
		}

		bool VKContext::HasStencilComponent(VkFormat format)
		{
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}
    	// Backend
		VkImageView VKBackend::CreateImageView(VkImage _tImage, VkFormat _format, VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT)
		{
			VkImageView tImageView;
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = _tImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = _format;
			viewInfo.subresourceRange.aspectMask = _aspectMask;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			VK_ASSERT(vkCreateImageView(g_context.m_LogicDevice, &viewInfo, nullptr, &tImageView));
			return tImageView;
		}
		void VKBackend::CreateImageViews()
		{
			/// Ahora vamos a crear las vistas a la imagenes, para poder acceder a ellas y demas
			m_SwapChainImagesViews.resize(m_SwapchainImagesCount);
			int currentSwapchaingImageView = 0;
			for (auto& image : m_SwapChainImages)
			{
				m_SwapChainImagesViews[currentSwapchaingImageView] = CreateImageView(image, m_SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
				++currentSwapchaingImageView;
			}
		}
		void VKBackend::Init()
		{
			uint32_t mExtensionCount = 0;
			const char** mExtensions;
			glm::mat4 m_matrix;
			glm::vec4 m_vec;
			const int m_Width = 1280;
			const int m_Height = 720;
			// 	cgltf_data* m_ModelData;
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
			m_Window = glfwCreateWindow(m_Width, m_Height, "Vulkan test", nullptr, nullptr);
			glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
			glfwSetKeyCallback(m_Window, KeyboardInputCallback);
			glfwSetCursorPosCallback(m_Window, MouseInputCallback);
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetMouseButtonCallback(m_Window, MouseBPressCallback);
			// INICIALIZAMOS IMGUI
			// IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui_ImplGlfw_InitForVulkan(m_Window, true);
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
#if 0
			// sprintf(modelPath, "resources/Models/SciFiHelmet/glTF/");
			// sprintf(modelName, "SciFiHelmet.gltf");
			// LoadModel(modelPath, modelName);
			/*sprintf(modelPath, "resources/Models/scene/glTF/");
			sprintf(modelName, "scene.gltf");
			LoadModel(modelPath, modelName );*/

#endif
			m_DbgModels.push_back(new R_DbgModel());

			/// Vamos a crear la integracion del sistema de ventanas (WSI) para vulkan
			// EXT: VK_KHR_surface
			if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
				exit(-2);

			// Present support on the Physical Device
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(g_context.m_GpuInfo.m_Device, g_context.m_GraphicsQueueFamilyIndex, m_Surface, &presentSupport);
			if (!presentSupport)
				printf("CANNOT PRESENT ON THIS DEVICE: %s\n", g_context.m_GpuInfo.m_Properties.deviceName);
			vkGetDeviceQueue(g_context.m_LogicDevice, g_context.m_GraphicsQueueFamilyIndex, 0, &g_context.m_PresentQueue);

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

			CreateSwapChain();
			vkGetSwapchainImagesKHR(g_context.m_LogicDevice, m_SwapChain, &m_SwapchainImagesCount, nullptr);
			m_SwapChainImages.resize(m_SwapchainImagesCount);
			vkGetSwapchainImagesKHR(g_context.m_LogicDevice, m_SwapChain, &m_SwapchainImagesCount, m_SwapChainImages.data());
			CreateImageViews();
			m_GraphicsRender = new GraphicsRenderer(g_context.m_LogicDevice);
			// primero creamos el layout de los descriptors
			m_GraphicsRender->CreateDescriptorSetLayout();
			m_DbgRender = new DebugRenderer(g_context.m_LogicDevice);
			m_DbgRender->CreateDescriptorSetLayout();
			for (auto& model : m_StaticModels)
			{
				for (auto& mesh : model->m_Meshes)
				{
					// Descriptor Pool
					model->m_Materials[mesh->m_Material]->CreateDescriptorPool(g_context.m_LogicDevice);
					// ahora creamos los Descriptor Sets en cada mesh
					model->m_Materials[mesh->m_Material]->CreateMeshDescriptorSet(g_context.m_LogicDevice, m_GraphicsRender->m_DescSetLayout);
				}
			}

			for (auto& model : m_DbgModels)
			{
				// Descriptor Pool
				model->m_Material.CreateDescriptorPool(g_context.m_LogicDevice);
				// ahora creamos los Descriptor Sets en cada mesh
				model->m_Material.CreateMeshDescriptorSet(g_context.m_LogicDevice, m_DbgRender->m_DescSetLayout);
			}
			// Inicializar Renderer
			m_GraphicsRender->Initialize();
			// Setup de la PipelineLayout
			m_GraphicsRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			// CreamosLaPipelileLayout
			m_GraphicsRender->CreatePipelineLayout();
			// Crear Renderpass
			g_context.CreateRenderPass(&m_SwapChainCreateInfo);
			// Crear Pipeline
			m_GraphicsRender->CreatePipeline(g_context.m_RenderPass);
			// Limpiar ShaderModules
			m_GraphicsRender->CleanShaderModules();
			// Lo mismo para el renderer de Debug
			m_DbgRender->Initialize();
			m_DbgRender->CreatePipelineLayoutSetup(&m_CurrentExtent, &m_Viewport, &m_Scissor);
			m_DbgRender->CreatePipelineLayout();
			m_DbgRender->CreatePipeline(g_context.m_RenderPass);
			m_DbgRender->CleanShaderModules();

			vkGetPhysicalDeviceMemoryProperties(g_context.m_GpuInfo.m_Device, &m_Mem_Props);
			CreateCommandBuffer();
			// Creamos los recursos para el Depth testing
			CreateDepthTestingResources();
			CreateFramebuffers(m_DbgRender);
			// Crear Textures
			for (auto& model : m_StaticModels)
			{
				for (auto& mesh : model->m_Meshes)
				{
					CreateAndTransitionImage(model->m_Path, model->m_Materials[mesh->m_Material]->m_TextureDiffuse);
					CreateAndTransitionImage(model->m_Path, model->m_Materials[mesh->m_Material]->m_TextureSpecular);
					CreateAndTransitionImage(model->m_Path, model->m_Materials[mesh->m_Material]->m_TextureAmbient);
					// Vertex buffer
					void* data;
					if (mesh->m_Vertices.size() <= 0)
					{
						fprintf(stderr, "There is no Triangles to inser on the buffer");
						exit(-57);
					}
					VkDeviceSize bufferSize = sizeof(mesh->m_Vertices[0]) * mesh->m_Vertices.size();
					// Stagin buffer
					CreateBuffer(bufferSize,
						VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_StagingBuffer, m_StaggingBufferMemory);
					vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, bufferSize, 0, &data);
					memcpy(data, mesh->m_Vertices.data(), (size_t)bufferSize);
					vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
					CreateBuffer(bufferSize,
						VK_BUFFER_USAGE_TRANSFER_DST_BIT |
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						mesh->m_VertexBuffer, mesh->m_VertexBufferMemory);
					CopyBuffer(mesh->m_VertexBuffer, m_StagingBuffer, bufferSize);
					vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
					vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
					if (mesh->m_Indices.size() > 0)
					{
						// Index buffer
						bufferSize = sizeof(mesh->m_Indices[0]) * mesh->m_Indices.size();
						// Stagin buffer
						CreateBuffer(bufferSize,
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							m_StagingBuffer, m_StaggingBufferMemory);
						vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, bufferSize, 0, &data);
						memcpy(data, mesh->m_Indices.data(), (size_t)bufferSize);
						vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
						CreateBuffer(bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_SHARING_MODE_CONCURRENT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							mesh->m_IndexBuffer, mesh->m_IndexBufferMemory);
						CopyBuffer(mesh->m_IndexBuffer, m_StagingBuffer, bufferSize);
						vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
						vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
					}
				}
			}
			// Uniform buffers
			m_UniformBuffers.resize(FRAMES_IN_FLIGHT);
			m_UniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_Uniform_SBuffersMapped.resize(FRAMES_IN_FLIGHT);
			VkDeviceSize bufferSize = sizeof(UniformBufferObject);
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
			// Dynamic buffers
			m_DynamicBuffers.resize(FRAMES_IN_FLIGHT);
			m_DynamicBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_DynamicBuffersMapped.resize(FRAMES_IN_FLIGHT);
			VkDeviceSize dynBufferSize = m_StaticModels.size() * sizeof(DynamicBufferObject);
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
			// DEBUG UNIFORM BUFFERS
			for (auto& model : m_DbgModels)
			{
				// Vertex buffer
				void* data;
				if (model->m_Vertices.size() <= 0)
				{
					fprintf(stderr, "There is no Triangles to inser on the buffer");
					exit(-57);
				}
				VkDeviceSize bufferSize = sizeof(model->m_Vertices[0]) * model->m_Vertices.size();
				// Stagin buffer
				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_StagingBuffer, m_StaggingBufferMemory);
				vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, bufferSize, 0, &data);
				memcpy(data, model->m_Vertices.data(), (size_t)bufferSize);
				vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					model->m_VertexBuffer, model->m_VertexBufferMemory);
				CopyBuffer(model->m_VertexBuffer, m_StagingBuffer, bufferSize);
				vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
			}
			// Uniform buffers
			m_DbgUniformBuffers.resize(FRAMES_IN_FLIGHT);
			m_DbgUniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_DbgUniformBuffersMapped.resize(FRAMES_IN_FLIGHT);
			VkDeviceSize dbgBufferSize = sizeof(DebugUniformBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				CreateBuffer(dbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgUniformBuffers[i], m_DbgUniformBuffersMemory[i]);
				vkMapMemory(g_context.m_LogicDevice, m_DbgUniformBuffersMemory[i], 0,
					dbgBufferSize, 0, &m_DbgUniformBuffersMapped[i]);
			}
			// Dynamic buffers
			m_DbgDynamicBuffers.resize(FRAMES_IN_FLIGHT);
			m_DbgDynamicBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_DbgDynamicBuffersMapped.resize(FRAMES_IN_FLIGHT);
			VkDeviceSize dynDbgBufferSize = m_DbgModels.size() * sizeof(DynamicBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				CreateBuffer(dynDbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgDynamicBuffers[i], m_DbgDynamicBuffersMemory[i]);
				vkMapMemory(g_context.m_LogicDevice, m_DbgDynamicBuffersMemory[i], 0,
					dynDbgBufferSize, 0, &m_DbgDynamicBuffersMapped[i]);
			}
			// UI descriptor Pool
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			VK_ASSERT(vkCreateDescriptorPool(g_context.m_LogicDevice, &pool_info, nullptr, &m_UIDescriptorPool));

			// IMGUI
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = m_Instance;
			init_info.PhysicalDevice = g_context.m_GpuInfo.m_Device;
			init_info.Device = g_context.m_LogicDevice;
			init_info.QueueFamily = g_context.m_GraphicsQueueFamilyIndex;
			init_info.Queue = g_context.m_GraphicsQueue;
			init_info.PipelineCache = VK_NULL_HANDLE;
			init_info.DescriptorPool = m_UIDescriptorPool;
			init_info.Allocator = nullptr;
			init_info.MinImageCount = m_Capabilities.minImageCount;
			init_info.ImageCount = m_SwapchainImagesCount;
			init_info.CheckVkResultFn = nullptr;
			ImGui_ImplVulkan_Init(&init_info, g_context.m_RenderPass);
			// Update DescriptsSets
			for (auto& model : m_StaticModels)
			{
				for (auto& mesh : model->m_Meshes)
				{
					model->m_Materials[mesh->m_Material]->UpdateDescriptorSet(g_context.m_LogicDevice, m_UniformBuffers, m_DynamicBuffers);
				}
			}
			//Debug Update DescriptorSet
			for (auto& model : m_DbgModels)
			{
				model->m_Material.UpdateDescriptorSet(g_context.m_LogicDevice, m_DbgUniformBuffers, m_DbgDynamicBuffers);
			}

			CreateSyncObjects(0);
			CreateSyncObjects(1);

			// Vamos a pre-ordenar los modelos para pintarlos segun el material.
			// BUBBLESORT de primeras, luego ya veremos, al ser tiempo pre-frameloop, no deberia importar.
			for (auto& model : m_StaticModels)
			{
				for (int i = 0; i < model->m_Meshes.size(); i++)
				{
					for (int j = 1; j < model->m_Meshes.size(); j++)
					{
						auto& mesh = model->m_Meshes[i];
						if (model->m_Meshes[j]->m_Material > model->m_Meshes[i]->m_Material)
						{
							auto tempMesh = model->m_Meshes[j];
							model->m_Meshes[j] = model->m_Meshes[i];
							model->m_Meshes[i] = tempMesh;
						}
					}
				}
			}
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
			_createInfo->enabledLayerCount = g_context.m_GpuInfo.m_ValidationLayers.size();
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
		}

		void VKBackend::RecreateSwapChain()
		{
			printf("\n\tRe-create Swapchain\n");
			// Si estamos minimizados, esperamos pacientemente a que se vuelva a ver la ventana
			int width = 0, height = 0;
			glfwGetFramebufferSize(m_Window, &width, &height);
			while (width == 0 || height == 0) {
				glfwGetFramebufferSize(m_Window, &width, &height);
				glfwWaitEvents();
			}
			vkDeviceWaitIdle(g_context.m_LogicDevice);
			// Esperamos a que termine de pintarse y recreamos la swapchain con los nuevos parametros
			CleanSwapChain();
			CreateSwapChain();
			vkGetSwapchainImagesKHR(g_context.m_LogicDevice, m_SwapChain, &m_SwapchainImagesCount, nullptr);
			m_SwapChainImages.resize(m_SwapchainImagesCount);
			vkGetSwapchainImagesKHR(g_context.m_LogicDevice, m_SwapChain, &m_SwapchainImagesCount, m_SwapChainImages.data());
			CreateImageViews();
			// CreateTextureImageView();
			// CreateTextureSampler();
			CreateFramebuffers(m_GraphicsRender);
			m_NeedToRecreateSwapchain = false;
		}

		void VKBackend::CreateFramebuffers(Renderer* _renderer)
		{
			// FRAMEBUFFERS
			m_SwapChainFramebuffers.resize(m_SwapChainImagesViews.size());
			for (size_t i = 0; i < m_SwapChainImagesViews.size(); i++)
			{
				std::array<VkImageView, 2> attachments = {
					m_SwapChainImagesViews[i],
					m_DepthImageView
				};
				VkFramebufferCreateInfo mFramebufferInfo{};
				mFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				mFramebufferInfo.renderPass = g_context.m_RenderPass;
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

		void VKBackend::CreateCommandBuffer()
		{
			// COMMAND BUFFERS
			/// Command Pool
			///	m_GraphicsQueueFamilyIndex
			VkCommandPoolCreateInfo m_PoolInfo{};
			m_PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			m_PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			m_PoolInfo.queueFamilyIndex = g_context.m_GraphicsQueueFamilyIndex;
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


		void VKBackend::CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode, VkMemoryPropertyFlags _memFlags, VkBuffer& buffer_,
			VkDeviceMemory& bufferMem_)
		{
			/*
				Create Vertex Buffers
			 */
			unsigned int queueFamilyIndices[] = { g_context.m_GraphicsQueueFamilyIndex, g_context.m_TransferQueueFamilyIndex };
			VkBufferCreateInfo mBufferInfo{};
			mBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			mBufferInfo.size = _size;
			mBufferInfo.usage = _usage;
			mBufferInfo.sharingMode = _shareMode;
			mBufferInfo.pQueueFamilyIndices = queueFamilyIndices;
			mBufferInfo.queueFamilyIndexCount = 2;

			if (vkCreateBuffer(g_context.m_LogicDevice, &mBufferInfo, nullptr, &buffer_) != VK_SUCCESS)
				exit(-6);
			VkMemoryRequirements mem_Requ;
			vkGetBufferMemoryRequirements(g_context.m_LogicDevice, buffer_, &mem_Requ);
			// WTF que hace esto solo aqui si se necesita para todo
			// vkGetPhysicalDeviceMemoryProperties(g_context.m_GpuInfo.m_Device, &m_Mem_Props);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = mem_Requ.size;
			allocInfo.memoryTypeIndex = g_context.m_GpuInfo.FindMemoryType(mem_Requ.memoryTypeBits, _memFlags);
			if (vkAllocateMemory(g_context.m_LogicDevice, &allocInfo, nullptr, &bufferMem_) != VK_SUCCESS)
				exit(-8);

			vkBindBufferMemory(g_context.m_LogicDevice, buffer_, bufferMem_, 0);
		}

		void VKBackend::CreateAndTransitionImage(char* _modelPath, Texture* _texture)
		{
			// Como utilizamos lightview, las texturas solo se crean una vez.
			if (_texture->tImageView == nullptr && _texture->m_Sampler == nullptr
				&& _texture->tImage == nullptr && _texture->tImageMem == nullptr)
			{
				int tWidth, tHeight, tChannels;
				stbi_uc* pixels;
				VkDeviceSize tSize;
				char textPath[512];
				sprintf(textPath, "%s%s", _modelPath, _texture->sPath.c_str());
				pixels = stbi_load(textPath, &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
				if (!pixels)
				{
					printf("\rMissing Texture %s\n", textPath);
					stbi_uc* m_DefaultTexture;
					m_DefaultTexture = stbi_load("resources/Textures/checkerW.png", &m_DefualtWidth, &m_DefualtHeight, &m_DefualtChannels, STBI_rgb_alpha);
					pixels = m_DefaultTexture;
					tWidth = m_DefualtWidth;
					tHeight = m_DefualtHeight;
				}
				tSize = tWidth * tHeight * 4;
				// Buffer transfer of pixels
				CreateBuffer(tSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_StagingBuffer, m_StaggingBufferMemory);
				void* data;
				vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, tSize, 0, &data);
				memcpy(data, pixels, (size_t)tSize);
				vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
				stbi_image_free(pixels);

				auto texSize = tWidth * tHeight * 4;
				CreateImage(tWidth, tHeight, VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_texture->tImage, &_texture->tImageMem);
				vkBindImageMemory(g_context.m_LogicDevice, _texture->tImage, _texture->tImageMem, 0);
				TransitionImageLayout(_texture->tImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				CopyBufferToImage(m_StagingBuffer, _texture->tImage, static_cast<uint32_t>(tWidth), static_cast<uint32_t>(tHeight), 0);
				TransitionImageLayout(_texture->tImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				_texture->tImageView = CreateTextureImageView(_texture->tImage);
				_texture->m_Sampler = CreateTextureSampler();
				vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
			}
		}

		void VKBackend::CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _w, uint32_t _h, VkDeviceSize _bufferOffset = 0)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();
			VkBufferImageCopy region{};
			region.bufferOffset = _bufferOffset;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { _w, _h, 1 };
			vkCmdCopyBufferToImage(commandBuffer, _buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			EndSingleTimeCommandBuffer(commandBuffer);
		}

		void VKBackend::CreateImage(unsigned int _Width, unsigned int _Height, VkFormat _format, VkImageTiling _tiling,
			VkImageUsageFlagBits _usage, VkMemoryPropertyFlags _memProperties,
			VkImage* _image, VkDeviceMemory* _imageMem)
		{
			VkImageCreateInfo tImageInfo{};
			tImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			tImageInfo.imageType = VK_IMAGE_TYPE_2D;
			tImageInfo.extent.width = static_cast<uint32_t>(_Width);
			tImageInfo.extent.height = static_cast<uint32_t>(_Height);
			tImageInfo.extent.depth = 1;
			tImageInfo.mipLevels = 1;
			tImageInfo.arrayLayers = 1;
			tImageInfo.format = _format;
			tImageInfo.tiling = _tiling;
			tImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			tImageInfo.usage = _usage;
			tImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			tImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			tImageInfo.flags = 0;
			VK_ASSERT(vkCreateImage(g_context.m_LogicDevice, &tImageInfo, nullptr, _image));
			VkMemoryRequirements memRequ;
			vkGetImageMemoryRequirements(g_context.m_LogicDevice, *_image, &memRequ);
			VkMemoryAllocateInfo tAllocInfo{};
			tAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			tAllocInfo.allocationSize = memRequ.size;
			tAllocInfo.memoryTypeIndex = g_context.m_GpuInfo.FindMemoryType(memRequ.memoryTypeBits, _memProperties);
			VK_ASSERT(vkAllocateMemory(g_context.m_LogicDevice, &tAllocInfo, nullptr, _imageMem));
		}

		void VKBackend::CreateDepthTestingResources()
		{
			VkFormat depthFormat = g_context.m_GpuInfo.FindDepthTestFormat();
			CreateImage(m_CurrentExtent.width, m_CurrentExtent.height, depthFormat,
				VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&m_DepthImage, &m_DepthImageMemory);
			// Transicionamos la imagen
			vkBindImageMemory(g_context.m_LogicDevice, m_DepthImage, m_DepthImageMemory, 0);
			TransitionImageLayout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			m_DepthImageView = CreateImageView(m_DepthImage, depthFormat,
				VK_IMAGE_ASPECT_DEPTH_BIT);
		}

		VkImageView VKBackend::CreateTextureImageView(VkImage _tImage)
		{
			return CreateImageView(_tImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		VkSampler VKBackend::CreateTextureSampler()
		{
			VkPhysicalDeviceProperties deviceProp;
			VkSamplerCreateInfo samplerInfo{};
			VkSampler TextureSampler;
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			vkGetPhysicalDeviceProperties(g_context.m_GpuInfo.m_Device, &deviceProp);
			samplerInfo.maxAnisotropy = deviceProp.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.f;
			samplerInfo.minLod = 0.f;
			samplerInfo.maxLod = 0.f;
			VK_ASSERT(vkCreateSampler(g_context.m_LogicDevice, &samplerInfo, nullptr, &TextureSampler));
			return TextureSampler;
		}

		VkCommandBuffer VKBackend::BeginSingleTimeCommandBuffer()
		{
			VkCommandBufferAllocateInfo mAllocInfo{};
			mAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			mAllocInfo.commandPool = m_CommandPool;
			mAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			mAllocInfo.commandBufferCount = 1;
			VkCommandBuffer commandBuffer;
			if (vkAllocateCommandBuffers(g_context.m_LogicDevice, &mAllocInfo, &commandBuffer) != VK_SUCCESS)
				exit(-12);
			VkCommandBufferBeginInfo mBeginInfo{};
			mBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			mBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			if (vkBeginCommandBuffer(commandBuffer, &mBeginInfo) != VK_SUCCESS)
				exit(-13);
			return commandBuffer;
		}
		void VKBackend::EndSingleTimeCommandBuffer(VkCommandBuffer _commandBuffer)
		{
			VK_ASSERT(vkEndCommandBuffer(_commandBuffer));
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &_commandBuffer;

			vkQueueSubmit(g_context.m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(g_context.m_GraphicsQueue);
			vkFreeCommandBuffers(g_context.m_LogicDevice, m_CommandPool, 1, &_commandBuffer);

		}
		void VKBackend::CopyBuffer(VkBuffer dst_, VkBuffer _src, VkDeviceSize _size)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();
			// Copiar desde el Stagging buffer al buffer
			VkBufferCopy copyRegion{};
			copyRegion.size = _size;
			vkCmdCopyBuffer(commandBuffer, _src, dst_, 1, &copyRegion);
			EndSingleTimeCommandBuffer(commandBuffer);
		}
		void VKBackend::TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _old, VkImageLayout _new)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();
			VkImageMemoryBarrier iBarrier{};
			iBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			iBarrier.oldLayout = _old;
			iBarrier.newLayout = _new;
			iBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.image = _image;
			if (_new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (g_context.HasStencilComponent(_format))
				{
					iBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else
				iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			iBarrier.subresourceRange.baseMipLevel = 0;
			iBarrier.subresourceRange.levelCount = 1;
			iBarrier.subresourceRange.baseArrayLayer = 0;
			iBarrier.subresourceRange.layerCount = 1;
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
			EndSingleTimeCommandBuffer(commandBuffer);
		}

		void VKBackend::EditorLoop()
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("Tools");
			{
				float tempLightPos[3];
				ImGui::LabelText("Light Pos", "Light Pos(%.2f, %.2f, %.2f)", m_LightPos.x, m_LightPos.y, m_LightPos.z);
				tempLightPos[0] = m_LightPos.x;
				tempLightPos[1] = m_LightPos.y;
				tempLightPos[2] = m_LightPos.z;
				ImGui::SliderFloat3("Light Position", tempLightPos, -10.0f, 10.0f);
				m_LightPos.x = tempLightPos[0];
				m_LightPos.y = std::max(0.0f, tempLightPos[1]);
				m_LightPos.z = tempLightPos[2];
				// Camera
				ImGui::SliderFloat("cam Speed", &m_CameraSpeed, 0.1f, 100.f);
				ImGui::Checkbox("Indexed Draw", &m_IndexedRender);
				ImGui::LabelText("Cam Pos", "Cam Pos(%.2f, %.2f, %.2f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
				ImGui::LabelText("Cam Pitch", "Cam Pitch(%.2f)", m_CameraPitch);
				ImGui::LabelText("Cam Yaw", "Cam Yaw(%.2f)", m_CameraYaw);

				if (ImGui::Button("Center Model"))
				{
					m_StaticModels[0]->m_Pos = glm::vec3(0.0f);
				}
				if (ImGui::Button("Center Light"))
				{
					m_LightPos = glm::vec3(0.0f);
				}
				ImGui::End();
			}
			ImGui::Begin("Lighting");
			{
				float color[3];
				color[0] = m_LightColor.x;
				color[1] = m_LightColor.y;
				color[2] = m_LightColor.z;
				ImGui::ColorEdit3("Color", color);
				m_LightColor.x = color[0];
				m_LightColor.y = color[1];
				m_LightColor.z = color[2];
				ImGui::End();
			}
			ImGui::Begin("DEBUG PANEL");
			{
				ImGui::Text("%s", g_ConsoleMSG.c_str());
				ImGui::End();
			}
			ImGui::EndFrame();
		}

		void VKBackend::RecordCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIdx, unsigned int _frameIdx,
			Renderer* _renderer, ImDrawData* draw_data)
		{
			// Record command buffer
			VkCommandBufferBeginInfo mBeginInfo{};
			mBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			mBeginInfo.flags = 0;
			mBeginInfo.pInheritanceInfo = nullptr;
			if (vkBeginCommandBuffer(_commandBuffer, &mBeginInfo) != VK_SUCCESS)
				exit(-13);
			// Clear Color
			std::array<VkClearValue, 2> clearValues;
			clearValues[0].color = defaultClearColor;
			clearValues[1].depthStencil = { 1.0f, 0 };
			// Render pass
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = g_context.m_RenderPass;
			renderPassInfo.framebuffer = m_SwapChainFramebuffers[_imageIdx];
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = m_CurrentExtent;
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			// Drawing Commands
			vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _renderer->m_Pipeline);
			// REFRESH RENDER MODE FUNCTIONS
			vkCmdSetViewport(_commandBuffer, 0, 1, &m_Viewport);
			vkCmdSetScissor(_commandBuffer, 0, 1, &m_Scissor);

			auto dynamicAlignment = sizeof(glm::mat4);
			if (g_context.m_GpuInfo.minUniformBufferOffsetAlignment > 0)
			{
				dynamicAlignment = (dynamicAlignment + g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1) & ~(g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			}
			uint32_t count = 0;
			for (auto& model : m_StaticModels)
			{
				DynamicBufferObject dynO{};
				dynO.model = glm::mat4(1.0f);
				dynO.model = glm::translate(dynO.model, model->m_Pos);
				uint32_t dynamicOffset = count * static_cast<uint32_t>(dynamicAlignment);
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)m_DynamicBuffersMapped[m_CurrentLocalFrame] + dynamicOffset, &dynO, sizeof(dynO));
				for (auto& mesh : model->m_Meshes)
				{
					// Update Uniform buffers

					VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer };
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _renderer->m_PipelineLayout, 0, 1,
						&model->m_Materials[mesh->m_Material]->m_DescriptorSet[m_CurrentLocalFrame], 1, &dynamicOffset);
					vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertesBuffers, offsets);
					vkCmdBindIndexBuffer(_commandBuffer, mesh->m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
					// Draw Loop
					if (m_IndexedRender && mesh->m_Indices.size() > 0)
					{
						vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(mesh->m_Indices.size()), 1, 0, 0, 0);
					}
					else
					{
						vkCmdDraw(_commandBuffer, mesh->m_Vertices.size(), 1, 0, 0);
					}
					// Flush to make changes visible to the host
					VkMappedMemoryRange mappedMemoryRange{};
					mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
					mappedMemoryRange.memory = m_DynamicBuffersMemory[m_CurrentLocalFrame];
					mappedMemoryRange.size = sizeof(dynO);
					vkFlushMappedMemoryRanges(g_context.m_LogicDevice, 1, &mappedMemoryRange);
				}
				++count;
			}
			// ---- Draw Loop
			// DEBUG Render
			vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DbgRender->m_Pipeline);
			// REFRESH RENDER MODE FUNCTIONS
			vkCmdSetViewport(_commandBuffer, 0, 1, &m_Viewport);
			vkCmdSetScissor(_commandBuffer, 0, 1, &m_Scissor);
			int debugCount = 0;
			m_DbgModels[0]->m_Pos = m_LightPos;
			for (auto& model : m_DbgModels)
			{
				DynamicBufferObject dynO{};
				dynO.model = glm::mat4(1.0f);
				dynO.model = glm::translate(dynO.model, model->m_Pos);
				uint32_t dynamicOffset = debugCount * static_cast<uint32_t>(dynamicAlignment);
				VkBuffer vertesBuffers[] = { model->m_VertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)m_DbgDynamicBuffersMapped[m_CurrentLocalFrame] + dynamicOffset, &dynO, sizeof(dynO));
				vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DbgRender->m_PipelineLayout, 0, 1, &model->m_Material.m_DescriptorSet[m_CurrentLocalFrame], 1, &dynamicOffset);
				vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertesBuffers, offsets);
				vkCmdDraw(_commandBuffer, model->m_Vertices.size(), 1, 0, 0);
				++debugCount;
			}
			// UI Render
			if (draw_data)
				ImGui_ImplVulkan_RenderDrawData(draw_data, _commandBuffer);
			vkCmdEndRenderPass(_commandBuffer);
			if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
				exit(-17);
		}

		void VKBackend::DrawFrame()
		{
			// Esperamos por el frame anterior y reseteamos la Fence
			vkWaitForFences(g_context.m_LogicDevice, 1, &m_InFlight[m_CurrentLocalFrame], VK_TRUE, UINT64_MAX);
			vkResetFences(g_context.m_LogicDevice, 1, &m_InFlight[m_CurrentLocalFrame]);
			vkResetCommandBuffer(m_CommandBuffer[m_CurrentLocalFrame], 0);
			uint32_t imageIdx;
			auto acqResult = vkAcquireNextImageKHR(g_context.m_LogicDevice, m_SwapChain, UINT64_MAX, m_ImageAvailable[m_CurrentLocalFrame],
				VK_NULL_HANDLE, &imageIdx);
			if (acqResult == VK_ERROR_OUT_OF_DATE_KHR || acqResult == VK_SUBOPTIMAL_KHR)
				RecreateSwapChain();
			else if (acqResult != VK_SUCCESS && acqResult != VK_SUBOPTIMAL_KHR)
				exit(-69);
			// Update Uniform buffers
			UniformBufferObject ubo{};
			ubo.view = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward,
				m_CameraUp);
			ubo.projection = glm::perspective(glm::radians(m_CameraFOV), 1.0f, 0.1f, 1000000.f);
			ubo.projection[1][1] *= -1; // para invertir el eje Y
			ubo.cameraPosition = m_CameraPos;
			ubo.lightPosition = m_LightPos;
			ubo.lightColor = m_LightColor;
			memcpy(m_Uniform_SBuffersMapped[m_CurrentLocalFrame], &ubo, sizeof(ubo));

			//Debug
			DebugUniformBufferObject dubo{};
			dubo.view = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward,
				m_CameraUp);
			dubo.projection = glm::perspective(glm::radians(m_CameraFOV), 1.0f, 0.1f, 1000000.f);

			memcpy(m_DbgUniformBuffersMapped[m_CurrentLocalFrame], &dubo, sizeof(dubo));

			// Render ImGui
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			RecordCommandBuffer(m_CommandBuffer[m_CurrentLocalFrame], imageIdx, m_CurrentLocalFrame, m_GraphicsRender, draw_data);
			// RecordDebugCommandBuffer(m_CommandBuffer[m_CurrentLocalFrame], imageIdx, m_CurrentLocalFrame, m_DebugPipeline);
			// UIRender(m_CommandBuffer[m_CurrentLocalFrame], imageIdx, m_CurrentLocalFrame, draw_data);
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			VkSemaphore waitSemaphores[] = { m_ImageAvailable[m_CurrentLocalFrame] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_CommandBuffer[m_CurrentLocalFrame];
			VkSemaphore signalSemaphores[] = { m_RenderFinish[m_CurrentLocalFrame] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			if (vkQueueSubmit(g_context.m_GraphicsQueue, 1, &submitInfo, m_InFlight[m_CurrentLocalFrame]) != VK_SUCCESS)
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
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIdx;
			presentInfo.pResults = nullptr;
			m_PresentResult = vkQueuePresentKHR(g_context.m_PresentQueue, &presentInfo);
			if ((m_PresentResult == VK_ERROR_OUT_OF_DATE_KHR || m_PresentResult == VK_SUBOPTIMAL_KHR)
				&& m_NeedToRecreateSwapchain)
				RecreateSwapChain();
			else if (m_PresentResult != VK_SUCCESS && m_PresentResult != VK_SUBOPTIMAL_KHR)
				exit(-69);
		}

		void VKBackend::CleanSwapChain()
		{
			for (auto& framebuffer : m_SwapChainFramebuffers)
				vkDestroyFramebuffer(g_context.m_LogicDevice, framebuffer, nullptr);
			for (auto& imageView : m_SwapChainImagesViews)
				vkDestroyImageView(g_context.m_LogicDevice, imageView, nullptr);
			vkDestroySwapchainKHR(g_context.m_LogicDevice, m_SwapChain, nullptr);
		}

		void VKBackend::ProcessModelNode(aiNode* _node, const aiScene* _scene)
		{
			// CHILDREN
			for (unsigned int i = 0; i < _node->mNumChildren; i++)
			{
				ProcessModelNode(_node->mChildren[i], _scene);
			}
			int lastTexIndex = 0;
			uint32_t tempMaterial = -1;
			for (int m = 0; m < _node->mNumMeshes; m++)
			{
				const aiMesh* mesh = _scene->mMeshes[_node->mMeshes[m]];
				R_Mesh* tempMesh = new R_Mesh();
				//Process Mesh
				for (unsigned int f = 0; f < mesh->mNumFaces; f++)
				{
					const aiFace& face = mesh->mFaces[f];
					for (unsigned int j = 0; j < face.mNumIndices; j++)
					{
						// m_Indices.push_back(face.mIndices[0]);
						// m_Indices.push_back(face.mIndices[1]);
						// m_Indices.push_back(face.mIndices[2]);
						tempMesh->m_Indices.push_back(face.mIndices[0]);
						tempMesh->m_Indices.push_back(face.mIndices[1]);
						tempMesh->m_Indices.push_back(face.mIndices[2]);
					}
				}
				for (unsigned int v = 0; v < mesh->mNumVertices; v++)
				{
					Vertex3D tempVertex;
					tempVertex.m_Pos = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z };
					if (mesh->mTextureCoords[0])
					{
						tempVertex.m_TexCoord = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
					}
					else
					{
						tempVertex.m_TexCoord = { 0.f, 0.f };
					}
					tempVertex.m_Normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
					// New New Mexico
					tempMesh->m_Vertices.push_back(tempVertex);
				}
				// Textura por Mesh
				int texIndex = 0;
				aiString path;
				if (tempModel->m_Materials[mesh->mMaterialIndex] == nullptr)
				{
					// printf("\tNew Material %d\n", mesh->mMaterialIndex);
					tempModel->m_Materials[mesh->mMaterialIndex] = new R_Material();
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureDiffuse = new Texture(std::string(path.data));
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_SPECULAR, texIndex, &path);
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureSpecular = new Texture(std::string(path.data));
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_AMBIENT, texIndex, &path);
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureAmbient = new Texture(std::string(path.data));
				}
				tempMesh->m_Material = mesh->mMaterialIndex;
				//++m_TotalTextures;
				tempModel->m_Meshes.push_back(tempMesh);
			}
		}

		void VKBackend::LoadModel(const char* _filepath, const char* _modelName)
		{
			char filename[128];
			sprintf(filename, "%s%s", _filepath, _modelName);
			printf("\nLoading %s\n", _modelName);
			const aiScene* scene = aiImportFile(filename, aiProcess_Triangulate);
			if (!scene->HasMeshes())
				exit(-225);
			tempModel = new R_Model();
			//Process Node
			auto node = scene->mRootNode;
			ProcessModelNode(node, scene);
			// Insert new static model
			sprintf(tempModel->m_Path, _filepath, 64);
			m_StaticModels.push_back(tempModel);
		}
		bool VKBackend::BackendShouldClose()
		{
			return m_CloseEngine || glfwWindowShouldClose(m_Window);
		}

		void VKBackend::PollEvents()
		{
			glfwPollEvents();
		}

		void VKBackend::Cleanup()
		{
			printf("Cleanup\n");
			vkDeviceWaitIdle(g_context.m_LogicDevice);
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			vkDestroyDescriptorPool(g_context.m_LogicDevice, m_UIDescriptorPool, nullptr);
			vkDestroyRenderPass(g_context.m_LogicDevice, g_context.m_RenderPass, nullptr);
			delete m_GraphicsRender;
			delete m_DbgRender;
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				vkDestroyBuffer(g_context.m_LogicDevice, m_DynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_DynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_UniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_DbgDynamicBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_DbgDynamicBuffersMemory[i], nullptr);

				vkDestroyBuffer(g_context.m_LogicDevice, m_DbgUniformBuffers[i], nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_DbgUniformBuffersMemory[i], nullptr);
			}
			vkDestroySemaphore(g_context.m_LogicDevice, m_ImageAvailable[0], nullptr);
			vkDestroySemaphore(g_context.m_LogicDevice, m_ImageAvailable[1], nullptr);
			vkDestroySemaphore(g_context.m_LogicDevice, m_RenderFinish[0], nullptr);
			vkDestroySemaphore(g_context.m_LogicDevice, m_RenderFinish[1], nullptr);
			vkDestroyFence(g_context.m_LogicDevice, m_InFlight[0], nullptr);
			vkDestroyFence(g_context.m_LogicDevice, m_InFlight[1], nullptr);
			vkDestroyCommandPool(g_context.m_LogicDevice, m_CommandPool, nullptr);
			CleanSwapChain();
			for (auto& model : m_StaticModels)
			{
				for (auto& [idx, mat] : model->m_Materials)
				{
					mat->Cleanup(g_context.m_LogicDevice);
				}
				for (auto& mesh : model->m_Meshes)
				{
					mesh->Cleanup(g_context.m_LogicDevice);
				}
			}
			for (auto& model : m_DbgModels)
			{
				model->Cleanup(g_context.m_LogicDevice);
			}
			vkDestroyImageView(g_context.m_LogicDevice, m_DepthImageView, nullptr);
			vkDestroyImage(g_context.m_LogicDevice, m_DepthImage, nullptr);
			vkFreeMemory(g_context.m_LogicDevice, m_DepthImageMemory, nullptr);
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
