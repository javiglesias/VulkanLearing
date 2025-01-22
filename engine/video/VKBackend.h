#ifndef _C_BACKEND
#define _C_BACKEND

#include "VKRUtils.h"
#include "../core/VKRenderers.h"
#include "../core/Materials/VKRShadowMaterial.h"
#include "../core/Objects/VKRModel.h"
#include "../core/Objects/VKRLight.h"
#include <thread>

struct GLFWwindow;

namespace VKR
{
    namespace render
    {
#define MAX_MODELS 256
#define MAX_MESHES 1024
#define WIN_HEIGHT 720
#define WIN_WIDTH 1280

        enum G_PIPELINE_STATUS
        {
	        INVALID,
            CREATING,
            READY,
            DESTROYED
        };
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
        inline bool m_SceneDirty = false;
        inline bool g_DrawCubemap = true;
        inline double m_LastYPosition = 0.f, m_LastXPosition = 0.f;
        inline double m_CameraYaw = 0.f, m_CameraPitch = 0.f;
        inline float m_CameraSpeed = 0.6f;
        inline float m_CameraFOV = 70.f;
        inline float zFar= 1000001.f;
        inline float zNear = 0.1f;
        inline float g_debugScale = 1.f;
        inline float g_cubemapDistance = 1000000.f;
        inline float g_ShadowAR = 1.f;
        inline float g_ShadowBias = 0.0025f;
        inline float g_MipLevel = 0.f;
        inline float g_Rotation = 0.f;
		inline float g_TimestampValue = 0.f;
        inline double g_FrameTime;
        inline long long g_CurrentFrame = 0;
        inline double g_ElapsedTime;
		inline char* g_CommandLine;
		inline char* g_CommandLineHistory;

        inline GLFWwindow* m_Window;
        inline float m_ShadowCameraFOV = 45.f;
        inline std::string g_ConsoleMSG;
        inline glm::vec3 m_PointLightPos = glm::vec3(0.f, 3.f, 0.f);
        inline glm::vec3 m_PointOpts = glm::vec3(1.f, 3.f, 0.f);
        inline glm::vec3 m_LightColor = glm::vec3(1.f, 1.f, 0.f);
        inline glm::vec3 m_CameraPos = glm::vec3(0.f);
        inline glm::vec3 m_CameraForward = glm::vec3(0.f, 0.f, 1.f);
        inline glm::vec3 m_CameraUp = glm::vec3(0.f, 1.f, 0.f);
        inline glm::vec3 m_Rotation = glm::vec3(0.f, 1.f, 1.f);
		//inline GraphicsRenderer* m_GraphicsRender;
		inline ShadowRenderer* m_ShadowRender;
		inline ShaderRenderer* m_GridRender;
        inline DebugRenderer* m_DbgRender;
        inline CubemapRenderer* m_CubemapRender;
        inline std::vector<R_DbgModel*> m_DbgModels; // lights
        inline R_Model* m_StaticModels[MAX_MODELS];
        inline R_Model* m_PendingBuffersModels[MAX_MODELS];
		inline std::unordered_map<const char*, VkShaderModule> vert_shader_modules;
		inline std::unordered_map<const char*, VkShaderModule> frag_shader_modules;
        inline int m_CurrentStaticModels = 0;
        inline int m_CurrentPendingModels = 0;
        inline std::vector<Light*> g_Lights;
		inline std::vector<uint64_t> g_Timestamps{};
        inline Directional* g_DirectionalLight;
        inline Point* g_PointLights[4];

        class VKBackend
        {
            //Variables
        public:
            bool m_CubemapRendering = false;
            bool m_RenderInitialized = false;
            uint32_t m_LastImageIdx;
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
            std::vector<VkImage> m_SwapChainImages;
            std::vector<VkImageView> m_SwapChainImagesViews;
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

            // RENDER BUFFERS
            std::vector<VkBuffer> m_UniformBuffers;
            std::vector<VkDeviceMemory> m_UniformBuffersMemory;
            std::vector<void*> m_Uniform_SBuffersMapped;

            std::vector<VkBuffer> m_DynamicBuffers;
            std::vector<VkDeviceMemory> m_DynamicBuffersMemory;
            std::vector<void*> m_DynamicBuffersMapped;

            std::vector<VkBuffer> m_LightsBuffers;
            std::vector<VkDeviceMemory> m_LightsBuffersMemory;
            std::vector<void*> m_LightsBuffersMapped;
            
            // DEBUG BUFFERS
            std::vector<VkBuffer> m_DbgUniformBuffers;
            std::vector<VkDeviceMemory> m_DbgUniformBuffersMemory;
            std::vector<void*> m_DbgUniformBuffersMapped;
            std::vector<VkBuffer> m_DbgDynamicBuffers;
            std::vector<VkDeviceMemory> m_DbgDynamicBuffersMemory;
            std::vector<void*> m_DbgDynamicBuffersMapped;
            // SHADOW BUFFERS
            std::vector<VkBuffer> m_ShadowUniformBuffers;
            std::vector<VkDeviceMemory> m_ShadowUniformBuffersMemory;
            std::vector<VkBuffer> m_ShadowDynamicBuffers;
            std::vector<VkDeviceMemory> m_ShadowDynamicBuffersMemory;
            std::vector<void*> m_ShadowUniformBuffersMapped;
            std::vector<void*> m_ShadowDynamicBuffersMapped;
            // CUBEMAP BUFFERS
            std::vector<VkBuffer> m_CubemapUniformBuffers;
            std::vector<VkBuffer> m_CubemapDynamicBuffers;
            std::vector<VkDeviceMemory> m_CubemapUniformBuffersMemory;
            std::vector<VkDeviceMemory> m_CubemapDynamicBuffersMemory;
            std::vector<void*> m_CubemapUniformBuffersMapped;
            std::vector<void*> m_CubemapDynamicBuffersMapped;
            
            // GRID BUFFERS
            std::vector<VkBuffer> m_GridUniformBuffers;
            std::vector<VkDeviceMemory> m_GridUniformBuffersMemory;
            std::vector<void*> m_GridUniformBuffersMapped;

            VkBuffer m_StagingBuffer;
            VkDeviceMemory m_StaggingBufferMemory;

            VkPhysicalDeviceMemoryProperties m_Mem_Props;
            VkImage m_DepthImage;
            VkDeviceMemory m_DepthImageMemory;
            VkImageView m_DepthImageView;

            // Gbuffer
            VkImage m_GBufferImage;
            VkDeviceMemory m_GBufferImageMemory;
            VkImageView m_GBufferImageView;

            VkClearColorValue defaultClearColor = { { 1.f, 0.f, 0.f, 1.0f } };

            // Para tener mas de un Frame, cada frame debe tener su pack de semaforos y Fencesnot
            VkCommandBuffer m_CommandBuffer[FRAMES_IN_FLIGHT];
            VkSemaphore m_ImageAvailable[FRAMES_IN_FLIGHT];
            VkSemaphore m_RenderFinish[FRAMES_IN_FLIGHT];
            VkFence		m_InFlight[FRAMES_IN_FLIGHT];

            // Functions
        public:
            VKBackend() {}
            void Init();
            void GenerateDBGBuffers();
            void GenerateBuffers();
            void InitializeVulkan(VkApplicationInfo* _appInfo);
            void Loop();
            bool BackendShouldClose();
            void PollEvents();
            void BeginRenderPass(unsigned int _InFlightFrame);
            uint32_t BeginFrame(unsigned int _InFlightFrame);
            void EndRenderPass(unsigned _InFlightFrame);
            void SubmitAndPresent(unsigned _FrameToPresent, uint32_t* _imageIdx);
            void CollectGPUTimestamps(unsigned int _FrameToPresent);
            void Shutdown();
            void Cleanup();

            double GetTime();

        private:
            void CreateInstance(VkInstanceCreateInfo* _createInfo, VkApplicationInfo* _appInfo,
                                const char** m_Extensions,
                                uint32_t m_extensionCount);
            void CreateSwapChain();
            void RecreateSwapChain();
            void CreateFramebufferAndSwapchain();
            void CreateShadowFramebuffer();
            void CreateFramebuffers();
            void CreateCommandBuffer();
            void CreateSyncObjects(unsigned _frameIdx);
            void CreatePerformanceQueries();
            void CreateImageViews();
            void CreateShadowResources();
            void CreateDepthTestingResources();
            void CreateGBufferImage();
            void RecordCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIdx, unsigned int _frameIdx,
                                     Renderer* _renderer);
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