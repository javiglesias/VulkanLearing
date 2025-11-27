#ifndef _C_BACKEND
#define _C_BACKEND

#include "../core/VKRenderers.h"
#include "../core/Materials/VKRShadowMaterial.h"
#include "../core/Objects/VKRModel.h"
#include "../core/Objects/VKRLight.h"
#include <thread>
#include <Windows.h>
#include <unordered_map>
#define FRAMES_IN_FLIGHT 2

struct GLFWwindow;
class Texture;
struct VmaAllocator_T;
typedef struct VmaAllocation_T* VmaAllocation;

namespace VKR
{
    namespace render
    {
#define MAX_MODELS 256
#define MAX_MESHES 1024
#define WIN_HEIGHT 720
#define WIN_WIDTH 1280
#define N_LIGHTS 4

        enum G_PIPELINE_STATUS
        {
	        INVALID,
            CREATING,
            READY,
            DESTROYED
        };
        inline VmaAllocator_T* vma_allocator;
        inline std::thread* g_LoadDataThread;
        inline G_PIPELINE_STATUS m_GPipelineStatus{ INVALID };
        inline int g_WindowWidth = WIN_WIDTH;
        inline int g_WindowHeight = WIN_HEIGHT;
		inline int g_ToneMapping = 0;
        inline bool m_NeedToRecreateSwapchain = false;
		inline bool g_GPUTimestamp = false;
        inline bool m_MouseCaptured = false;
        inline bool m_CloseEngine = false;
        inline bool m_IndexedRender = true;
        inline bool m_DebugRendering = false;
        inline bool m_CreateTestModel = false;
        inline bool m_SceneDirty = true;
        inline bool m_UIDirty = true;
        inline bool g_DrawCubemap = true;
        inline bool g_ShadowPassEnabled = true;
        inline double m_LastYPosition = 0.f, m_LastXPosition = 0.f;
        inline float zFar= 1000001.f;
        inline float zNear = 0.1f;
        inline float g_debugScale = 1.f;
        inline float g_cubemapDistance = 100.f;
        inline char g_CubemapTexture[256] = { "resources/textures/cubemaps/office.png" };
        inline float g_ShadowAR = 1.f;
        inline float g_ShadowBias = 0.0025f;
        inline float g_MipLevel = 0.f;
        inline float g_Rotation = 0.f;
		inline float g_TimestampValue = 0.f;
        inline double g_FrameTime;
        inline long long g_Frames = 0;
        inline double g_ElapsedTime;
		inline char* g_CommandLine;
		inline char* g_CommandLineHistory;
		inline glm::mat4 g_ProjectionMatrix {1.f};
		inline glm::mat4 g_ViewMatrix {1.f};
#ifdef USE_GLFW
        inline GLFWwindow* m_Window;
#else
        inline HWND hwnd;
#endif
        inline float m_ShadowCameraFOV = 45.f;
        inline std::string g_ConsoleMSG;
        inline glm::vec3 m_PointLightPos = glm::vec3(0.f, 3.f, 0.f);
        inline glm::vec3 m_PointOpts = glm::vec3(1.f, 3.f, 0.f);
        inline glm::vec3 m_LightColor = glm::vec3(1.f, 1.f, 0.f);
        
		//inline GraphicsRenderer* m_GraphicsRender;
		inline ShadowRenderer* m_ShadowRender;
		inline ShaderRenderer* m_GridRender;
        inline DebugRenderer* m_DbgRender;
        inline CubemapRenderer* m_CubemapRender;
        inline QuadRenderer* m_QuadRender;
        inline std::vector<R_DbgModel*> m_DbgModels; // lights
        inline R_Model* m_StaticModels[MAX_MODELS];
        inline R_Model* m_PendingBuffersModels[MAX_MODELS];
		inline std::unordered_map<const char*, VkShaderModule> vert_shader_modules;
		inline std::unordered_map<const char*, VkShaderModule> frag_shader_modules;
        inline int m_CurrentStaticModels = 0;
        inline int m_CurrentPendingModels = 0;
		inline std::vector<uint64_t> g_Timestamps{};
        inline Directional* g_DirectionalLight;
        inline Point g_PointLights[N_LIGHTS];

        class VKBackend
        {
            //Variables
        public:
            bool m_CubemapRendering = false;
            bool m_RenderInitialized = false;
            uint32_t m_FrameToPresent;
            uint32_t m_CurrentDebugModelsToDraw = 1;
            int m_TotalTextures;
            VkResult m_PresentResult;
            VkInstance m_Instance;
            VkDebugUtilsMessengerEXT m_DebugMessenger{};
            std::vector<const char*> m_InstanceExtensions;
            VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
            VkSurfaceCapabilitiesKHR m_Capabilities;
            //Swapchains things
            unsigned int m_SwapchainImagesCount;
            VkSwapchainCreateInfoKHR m_SwapChainCreateInfo{};
            Texture* m_SwapchainImages[FRAMES_IN_FLIGHT];

        	std::vector<Texture*> m_TexturesCache;

            std::vector<VkFramebuffer> m_SwapChainFramebuffers;
            // SHADOW
            VkFramebuffer m_ShadowFramebuffer;
			Texture* m_ShadowTexture;
            R_ShadowMaterial* m_ShadowMat;

            VkSwapchainKHR m_SwapChain;
            // Surface Things
            VkSurfaceFormatKHR m_SurfaceFormat;
            VkSurfaceKHR m_Surface;
            VkExtent2D m_CurrentExtent;
            VkViewport m_Viewport{};
            VkRect2D m_Scissor{};

            VkDescriptorPool m_DescriptorPool;
            VkCommandPool m_CommandPool;

			VkQueryPool m_PerformanceQuery[FRAMES_IN_FLIGHT];

            VkDescriptorSet m_ShadowVisualizer;

            std::vector<VkBuffer> m_LightsBuffers;
            std::vector<VkDeviceMemory> m_LightsBuffersMemory;
            std::vector<void*> m_LightsBuffersMapped;

            std::vector<VkBuffer> m_QuadBuffers;
            std::vector<VkDeviceMemory> m_QuadBuffersMemory;
            std::vector<void*> m_QuadBuffersMapped;

            std::vector<VkBuffer> m_QuadDynBuffers;
            std::vector<VkDeviceMemory> m_QuadDynBuffersMemory;
            std::vector<void*> m_QuadDynBuffersMapped;
            
            VkBuffer m_StagingBuffer;
            VkDeviceMemory m_StaggingBufferMemory;

            VkPhysicalDeviceMemoryProperties m_Mem_Props;
            Texture* m_DepthTexture;

            VkClearColorValue defaultClearColor = { { 1.f, 0.f, 0.f, 1.0f } };

            // Para tener mas de un Frame, cada frame debe tener su pack de semaforos y Fencesnot
            VkCommandBuffer m_CommandBuffer[FRAMES_IN_FLIGHT];
            VkSemaphore m_ImageAvailable[FRAMES_IN_FLIGHT];
            VkSemaphore m_RenderFinish[FRAMES_IN_FLIGHT];
            VkFence		m_InFlight[FRAMES_IN_FLIGHT];
            // Functions
        public:
            VKBackend() {}
            void Init(
#ifndef USE_GLFW
                HINSTANCE hInstance
#endif
            );
            void GenerateDBGBuffers();
            void GenerateBuffer(size_t _sizeDynAl, VkBuffer* _buffers, VkDeviceMemory* _buffsMemory, void** _mapped);
            void GenerateBuffers();
            void InitializeVulkan(VkApplicationInfo* _appInfo);
            void Loop();
            bool BackendShouldClose();
            void PollEvents();
            void BeginRenderPass(unsigned int _InFlightFrame);
            void EndRenderPass(unsigned _InFlightFrame);
            void CollectGPUTimestamps(unsigned int _FrameToPresent);
            void Shutdown();
            void Cleanup();
            Texture* FindTexture(const char* _textPath);
            void VMA_Initialize(VkPhysicalDevice _gpu, VkDevice _LogicDevice, VkInstance _instance);
            void VMA_CreateBuffer(size_t _size, VkMemoryPropertyFlags _memProperties, VmaAllocation* allocation_, VkBuffer* buffer_);
            void VMA_CreateImage(VkMemoryPropertyFlags _memProperties, VkImageCreateInfo* _ImageCreateInfo
                , VkImage* Image_, VmaAllocation* Allocation_);
            void VMA_BindTextureMemory(VkImage _image, VmaAllocation _allocation);
            void VMA_DestroyImage(VkImage _image, VmaAllocation _allocation);
            void CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode, VkMemoryPropertyFlags _memFlags, VkBuffer& buffer_, VkDeviceMemory& bufferMem_);
            double GetTime();

            void CreateInstance(VkInstanceCreateInfo* _createInfo, VkApplicationInfo* _appInfo, uint32_t m_extensionCount);
            void CreateSwapChain();
            void RecreateSwapChain();
            void CreateFramebufferAndSwapchain();
            void CreateFramebuffers();
            void CreateShadowFramebuffer();
            void CreateCommandBuffer();
            void CreateSyncObjects(unsigned _frameIdx);
            void CreatePerformanceQueries();
            void CreateShadowResources();
            void CreateDepthTestingResources();
            void RecordCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIdx, unsigned int _frameIdx,
                                     Renderer* _renderer);
            VkCommandBuffer BeginSingleTimeCommandBuffer(VkCommandPool _CommandPool);
            void EndSingleTimeCommandBuffer(VkCommandBuffer _commandBuffer, VkCommandPool _CommandPool, VkQueue _queue);
            void CopyBuffer(VkBuffer dst_, VkBuffer _src, VkDeviceSize _size, VkCommandPool _CommandPool, VkQueue _queue);
            void LoadTexture(Texture*);
            void LoadCubemapTexture(Texture* _texture);
            void CreateImage(Texture* _texture, VkExtent3D _extent, VkFormat _format, VkImageTiling _tiling
                , VkImageUsageFlagBits _usage, VkMemoryPropertyFlags _memProperties
                , uint32_t _arrayLayers, VkImageCreateFlags _flags, uint8_t _mipmapLvls = 1);
            void CreateAndTransitionImage(Texture* _texture, VkCommandPool _CommandPool
                , VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB
                , VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
                , VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D
                , uint32_t _arrayLayers = 1, VkImageCreateFlags _flags = 0);
            void CreateAndTransitionImageNoMipMaps(Texture* _texture, VkCommandPool _CommandPool
                , VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB
                , VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
                , VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D
                , uint32_t _arrayLayers = 1, VkImageCreateFlags _flags = 0);
            void CreateAndTransitionImageCubemap(Texture* _texture, VkCommandPool _CommandPool
                , VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB
                , VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
                , VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D
                , uint32_t _arrayLayers = 1, VkImageCreateFlags _flags = 0);

            void GenerateMipmap(Texture* _texture, VkCommandPool _CommandPool, uint8_t _mipLevels);
            void TransitionImageLayout(Texture* _texture, VkImageLayout _old, VkImageLayout _new,
                VkCommandPool _CommandPool, uint32_t _layerCount, VkQueue* _queue,
                uint8_t _levelCount = 1);
            void CopyBufferToImage(Texture* _texture, VkBuffer _buffer, VkExtent3D _extent, VkDeviceSize _bufferOffset
                , VkCommandPool _CommandPool, VkQueue* _queue, uint32_t _layer = 0);
            void CreateImageView(Texture* _texture, VkImageAspectFlags _aspectMask, VkImageViewType _viewType
                , uint32_t _arrayLayers = 1, uint32_t _levelCount = 1);
            void CreateTextureImageView(Texture* _texture);
            void BindTextureMemory(Texture* _texture);
            void CreateTextureSampler(Texture* _texture, float _Mipmaps,
                VkSamplerAddressMode _u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VkSamplerAddressMode _v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VkSamplerAddressMode _w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
            void CreateShadowTextureSampler(Texture* _texture);
            void CleanTextureData(Texture* _texture, VkDevice _LogicDevice);
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
            void CleanSwapChain();
        };
        VKBackend& GetVKBackend();
    }
}

#endif