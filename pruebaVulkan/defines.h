
#include <vulkan/vulkan_core.h>
const int FRAMES_IN_FLIGHT = 2;

std::vector<DBG_Vertex3D> Dbg_Cube =
    {   // x     y     z
        {{-1.0, -1.0,  1.0}, {1.0f, 1.0f, 0.0f}}, // 1  left    First Strip
        {{-1.0,  1.0,  1.0}, {1.0f, 1.0f, 0.0f}},// 3
        {{-1.0, -1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 0
        {{-1.0,  1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 2
        {{ 1.0, -1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 4  back
        {{ 1.0,  1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 6
        {{ 1.0, -1.0,  1.0}, {1.0f, 1.0f, 0.0f}},// 5  right
        {{ 1.0,  1.0,  1.0}, {1.0f, 1.0f, 0.0f}},// 7
        {{ 1.0,  1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 6  top     Second Strip
        {{-1.0,  1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 2
        {{ 1.0,  1.0,  1.0}, {1.0f, 1.0f, 0.0f}},// 7
        {{-1.0,  1.0,  1.0}, {1.0f, 1.0f, 0.0f}},// 3
        {{ 1.0, -1.0,  1.0}, {1.0f, 1.0f, 0.0f}},// 5  front
        {{-1.0, -1.0,  1.0}, {1.0f, 1.0f, 0.0f}},// 1
        {{ 1.0, -1.0, -1.0}, {1.0f, 1.0f, 0.0f}},// 4  bottom
        {{-1.0, -1.0, -1.0},  {1.0f, 1.0f, 0.0f}}// 0
    };

std::vector<R_Model*> m_StaticModels;
R_Model* tempModel;

const char g_SponzaPath[] = {"resources/Models/Sponza/glTF/"};
const char g_ModelsPath[] = {"resources/Models/%s/glTF/%s.gltf"};

bool m_NeedToRecreateSwapchain = false;
bool m_MouseCaptured = false;
bool m_IndexedRender = true;
unsigned int m_GPUSelected = 0;
unsigned int m_GraphicsQueueFamilyIndex = 0;
unsigned int m_TransferQueueFamilyIndex = 0;
unsigned int m_CurrentLocalFrame = 0;
unsigned int m_SwapchainImagesCount;
int m_CullMode = 2;
std::string g_ConsoleMSG;
stbi_uc* m_DefaultTexture;
int m_TotalTextures;
int m_DefualtWidth, m_DefualtHeight, m_DefualtChannels;
float m_LastYPosition = 0.f, m_LastXPosition = 0.f;
float m_CameraYaw = 0.f, m_CameraPitch = 0.f;
float m_CameraSpeed = 0.1f;
glm::vec3 m_CameraPos = glm::vec3(0.f);
glm::vec3 m_LightPos = glm::vec3(1.f);
glm::vec3 m_CameraForward = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 m_CameraUp = glm::vec3(0.f, 1.f, 0.f);
const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
std::vector<VkDynamicState> m_DynamicStates = { 
    VK_DYNAMIC_STATE_VIEWPORT, 
    VK_DYNAMIC_STATE_SCISSOR, 
    VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
    VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE, 
    VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE
    };
std::vector<const char*> m_DeviceExtensions;
std::vector<const char*> m_InstanceExtensions;
std::vector< VkDeviceQueueCreateInfo> m_QueueCreateInfos;
std::vector<VkImage> m_SwapChainImages;
std::vector<VkImageView> m_SwapChainImagesViews;
std::vector<VkFramebuffer> m_SwapChainFramebuffers;
std::vector<VkPhysicalDevice> m_PhysicalDevices;
auto mBindingDescription = Vertex3D::getBindingDescription();
auto mAttributeDescriptions = Vertex3D::getAttributeDescriptions();
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
VkDescriptorSetLayout m_DescSetLayout;
VkDescriptorPool m_DescriptorPool;
VkDescriptorPool m_UIDescriptorPool;
std::vector<VkDescriptorSet> m_DescriptorSets;
VkPipelineLayout m_PipelineLayout;
VkPipelineLayout m_DebugPipelineLayout;
VkRenderPass m_RenderPass;
VkRenderPass m_UIRenderPass;
VkRenderPass m_DebugRenderPass;
VkPipeline m_GraphicsPipeline;
VkPipeline m_DebugPipeline;
VkCommandPool m_CommandPool;
VkQueue m_PresentQueue;
VkBuffer m_StagingBuffer;
std::vector<VkBuffer> m_UniformBuffers;
std::vector<VkDeviceMemory> m_UniformBuffersMemory;
std::vector<void*> m_Uniform_SBuffersMapped;
VkDeviceMemory m_StaggingBufferMemory;
VkPhysicalDeviceMemoryProperties m_Mem_Props;
VkImage m_TextureImage;
VkImageView m_TextureImageView;
VkDeviceMemory m_TextureImageMemory;
VkImage m_DepthImage;
VkDeviceMemory m_DepthImageMemory;
VkImageView m_DepthImageView;
VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };
int m_PolygonMode = VK_POLYGON_MODE_FILL;

// Para tener mas de un Frame, cada frame debe tener su pack de semaforos y Fencesnot
VkCommandBuffer m_CommandBuffer[FRAMES_IN_FLIGHT];
VkSemaphore m_ImageAvailable[FRAMES_IN_FLIGHT];
VkSemaphore m_RenderFinish[FRAMES_IN_FLIGHT];
VkFence		m_InFlight[FRAMES_IN_FLIGHT];
