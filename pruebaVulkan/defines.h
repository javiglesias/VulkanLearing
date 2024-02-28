#include <unordered_map>
#include <vulkan/vulkan_core.h>

#define VK_CHECK(_value) \
	if(_value != VK_SUCCESS) \
	{VK_ASSERT(false);}

#define VK_CHECK_RET(_value) \
	if(_value != VK_SUCCESS) \
	{VK_ASSERT(false); return _value;}

#define CHECK(_expression) \
	exit(-96);

inline static void VK_ASSERT(bool _check)
{
	if(_check) exit(-69);
}

std::vector<R_Model*> m_StaticModels;
std::vector<R_DbgModel*> m_DbgModels;
R_Model* tempModel;

const char g_SponzaPath[] = {"resources/Models/Sponza/glTF/"};
const char g_ModelsPath[] = {"resources/Models/%s/glTF/%s.gltf"};
uint32_t minUniformBufferOffsetAlignment = 0;
bool m_NeedToRecreateSwapchain = false;
bool m_MouseCaptured = false;
bool m_IndexedRender = true;
bool m_DepthTest = true;
bool m_DepthWrite = true;
unsigned int m_GPUSelected = 0;
unsigned int m_GraphicsQueueFamilyIndex = 0;
unsigned int m_TransferQueueFamilyIndex = 0;
unsigned int m_CurrentLocalFrame = 0;
unsigned int m_SwapchainImagesCount;
std::string g_ConsoleMSG;
stbi_uc* m_DefaultTexture;
int m_TotalTextures;
int m_DefualtWidth, m_DefualtHeight, m_DefualtChannels;
float m_LastYPosition = 0.f, m_LastXPosition = 0.f;
float m_CameraYaw = 0.f, m_CameraPitch = 0.f;
float m_CameraSpeed = 0.1f;
float m_NewFrame = 0.0f;
float m_AccumulatedTime = 0.0f;
float m_DeltaTime = 0.0f;
float m_CurrentFrame = 0.0f;
float m_FrameCap = 0.016f; // 60fps
float m_CameraFOV = 70.f;
glm::vec3 m_CameraPos = glm::vec3(1.f);
glm::vec3 m_LightPos = glm::vec3(1.f);
glm::vec3 m_LightRot = glm::vec3(0.f);
glm::vec3 m_LightColor = glm::vec3(1.f);
glm::vec3 m_CameraForward = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 m_CameraUp = glm::vec3(0.f, -1.f, 0.f);
const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
std::vector<VkDynamicState> m_DynamicStates = { 
    VK_DYNAMIC_STATE_VIEWPORT, 
    VK_DYNAMIC_STATE_SCISSOR
    };
std::vector<const char*> m_DeviceExtensions;
std::vector<const char*> m_InstanceExtensions;
std::vector< VkDeviceQueueCreateInfo> m_QueueCreateInfos;
std::vector<VkImage> m_SwapChainImages;
std::vector<VkImageView> m_SwapChainImagesViews;
std::vector<VkFramebuffer> m_SwapChainFramebuffers;
std::vector<VkPhysicalDevice> m_PhysicalDevices;
auto m_BindingDescription = Vertex3D::getBindingDescription();
auto m_AttributeDescriptions = Vertex3D::getAttributeDescriptions();
auto m_DbgBindingDescription = DBG_Vertex3D::getBindingDescription();
auto m_DbgAttributeDescriptions = DBG_Vertex3D::getAttributeDescriptions();

GLFWwindow* m_Window;
VkResult m_PresentResult;
VkInstance m_Instance;
VkDebugUtilsMessengerEXT m_DebugMessenger {};
VkDevice m_LogicDevice;
VkSurfaceKHR m_Surface;
VkSurfaceFormatKHR m_SurfaceFormat;
VkExtent2D m_CurrentExtent;
VkViewport m_Viewport {};
VkRect2D m_Scissor {};
VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
VkSurfaceCapabilitiesKHR m_Capabilities;
VkSwapchainCreateInfoKHR m_SwapChainCreateInfo{};
VkSwapchainKHR m_SwapChain;
VkQueue m_GraphicsQueue;
VkQueue m_TransferQueue;
VkDescriptorPool m_DescriptorPool;
VkDescriptorPool m_UIDescriptorPool;
VkCommandPool m_CommandPool;
VkQueue m_PresentQueue;
VkBuffer m_StagingBuffer;
std::vector<VkBuffer> m_UniformBuffers;
std::vector<VkDeviceMemory> m_UniformBuffersMemory;
std::vector<VkBuffer> m_DynamicBuffers;
std::vector<VkDeviceMemory> m_DynamicBuffersMemory;
std::vector<void*> m_Uniform_SBuffersMapped;
std::vector<void*> m_DynamicBuffersMapped;
// DEBUG BUFFERS
std::vector<VkBuffer> m_DbgUniformBuffers;
std::vector<VkDeviceMemory> m_DbgUniformBuffersMemory;
std::vector<VkBuffer> m_DbgDynamicBuffers;
std::vector<VkDeviceMemory> m_DbgDynamicBuffersMemory;
std::vector<void*> m_DbgUniformBuffersMapped;
std::vector<void*> m_DbgDynamicBuffersMapped;
//
VkDeviceMemory m_StaggingBufferMemory;
VkPhysicalDeviceMemoryProperties m_Mem_Props;
VkImage m_DepthImage;
VkDeviceMemory m_DepthImageMemory;
VkImageView m_DepthImageView;
VkClearColorValue defaultClearColor = { { 0.6f, 0.65f, 0.4f, 1.0f } };
int m_PolygonMode = VK_POLYGON_MODE_FILL;
int m_DbgPolygonMode = VK_POLYGON_MODE_LINE;
VkCullModeFlagBits m_CullMode = VK_CULL_MODE_BACK_BIT;
VkCullModeFlagBits m_DbgCullMode = VK_CULL_MODE_FRONT_BIT;

// Para tener mas de un Frame, cada frame debe tener su pack de semaforos y Fencesnot
VkCommandBuffer m_CommandBuffer[FRAMES_IN_FLIGHT];
VkSemaphore m_ImageAvailable[FRAMES_IN_FLIGHT];
VkSemaphore m_RenderFinish[FRAMES_IN_FLIGHT];
VkFence		m_InFlight[FRAMES_IN_FLIGHT];
