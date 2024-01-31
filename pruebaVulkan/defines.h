// raylib https://github.com/raysan5/raylib/blob/c133fee286cfb3dad2fbfa40ab61f968850fd031/src/rmodels.c#L4891
#define LOAD_ATTRIBUTE(accesor, numComp, dataType, dstPtr) \
    { \
        int n = 0; \
        dataType *buffer = (dataType *)accesor->buffer_view->buffer->data + accesor->buffer_view->offset/sizeof(dataType) + accesor->offset/sizeof(dataType); \
        for (unsigned int k = 0; k < accesor->count; k++) \
        {\
            for (int l = 0; l < numComp; l++) \
            {\
                dstPtr[numComp*k + l] = buffer[n + l];\
            }\
            n += (int)(accesor->stride/sizeof(dataType));\
        }\
	}\

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
	alignas(16) glm::vec3 cameraPosition;
	alignas(16) glm::vec3 lightPosition;
};
struct Vertex2D {
	glm::vec2 m_Pos;
	glm::vec3 m_Color;
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof (Vertex2D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {

		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2D, m_Pos);

		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex2D, m_Color);

		return attributeDescriptions;
	}
};

struct DBG_Vertex3D 
{
	glm::vec3 m_Pos;
	glm::vec3 m_Color;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof (DBG_Vertex3D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {

		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(DBG_Vertex3D, m_Pos);

		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(DBG_Vertex3D, m_Color);

		return attributeDescriptions;
	}
};

struct Vertex3D {
	glm::vec3 m_Pos;
	glm::vec3 m_Color;
	glm::vec2 m_TexCoord;
	glm::vec3  m_Normal;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof (Vertex3D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {

		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3D, m_Pos);

		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3D, m_Color);

		attributeDescriptions[2].binding  = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3D, m_TexCoord);

		attributeDescriptions[3].binding  = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex3D, m_Normal);

		return attributeDescriptions;
	}
};

const int FRAMES_IN_FLIGHT = 2;

const std::vector<Vertex2D> m_TriangleVertices =
{
	{{-0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
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

std::vector<Vertex3D> m_ModelTriangles;

struct Texture 
{
	std::string sPath;
	VkImage tImage;
	VkDeviceMemory tImageMem;
	VkImageView tImageView;
	Texture()
	{
		sPath[0] = 0;
	}
	Texture(std::string _path)
	{
		sPath= _path;
	}
};
std::unordered_map<unsigned int, Texture> m_ModelTextures;

const char g_SponzaPath[] = {"resources/Models/Sponza/glTF/"};
const char g_ModelsPath[] = {"resources/Models/%s/glTF/%s.gltf"};

std::vector<uint16_t> m_Indices =
{
// 	0, 1, 2, 2, 3, 0
};

bool m_NeedToRecreateSwapchain = false;
bool m_MouseCaptured = false;
bool m_IndexedRender = true;
unsigned int m_GPUSelected = 0;
unsigned int m_GraphicsQueueFamilyIndex = 0;
unsigned int m_TransferQueueFamilyIndex = 0;
unsigned int m_CurrentLocalFrame = 0;
unsigned int m_SwapchainImagesCount;
int m_CullMode = 1;
std::string g_ConsoleMSG;
float m_LastYPosition = 0.f, m_LastXPosition = 0.f;
float m_CameraYaw = 0.f, m_CameraPitch = 0.f;
float m_CameraSpeed = 0.1f;
glm::vec3 m_CameraPos = glm::vec3(0.f);
glm::vec3 m_LightPos = glm::vec3(1.f);
glm::vec3 m_CameraForward = glm::vec3(0.f, 0.f, 1.f);
glm::vec3 m_CameraUp = glm::vec3(0.f, 1.f, 0.f);
const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
std::vector<VkDynamicState> m_DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
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
VkBuffer m_VertexBuffer;
VkBuffer m_StagingBuffer;
VkBuffer m_IndexBuffer;
std::vector<VkBuffer> m_UniformBuffers;
std::vector<VkDeviceMemory> m_UniformBuffersMemory;
std::vector<void*> m_Uniform_SBuffersMapped;
VkDeviceMemory m_VertexBufferMemory;
VkDeviceMemory m_StaggingBufferMemory;
VkDeviceMemory m_IndexBufferMemory;
VkPhysicalDeviceMemoryProperties m_Mem_Props;
VkImage m_TextureImage;
VkImageView m_TextureImageView;
VkDeviceMemory m_TextureImageMemory;
VkSampler m_TextureSampler;

// Para tener mas de un Frame, cada frame debe tener su pack de semaforos y Fencesnot
VkCommandBuffer m_CommandBuffer[FRAMES_IN_FLIGHT];
VkSemaphore m_ImageAvailable[FRAMES_IN_FLIGHT];
VkSemaphore m_RenderFinish[FRAMES_IN_FLIGHT];
VkFence		m_InFlight[FRAMES_IN_FLIGHT];
