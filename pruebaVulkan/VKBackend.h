#pragma once
#include "VKRenderers.h"
#include "VKRModel.h"
#include "VKRShadowMaterial.h"

#include "../dependencies/imgui/misc/single_file/imgui_single_file.h"
#include "../dependencies/imgui/backends/imgui_impl_glfw.h"
#include "../dependencies/imgui/backends/imgui_impl_vulkan.h"

namespace VKR
{
    namespace render
    {

        inline const int FRAMES_IN_FLIGHT = 2;
        inline bool m_NeedToRecreateSwapchain = false;
        inline bool m_MouseCaptured = false;
        inline bool m_CloseEngine = false;
        inline float m_LastYPosition = 0.f, m_LastXPosition = 0.f;
        inline float m_CameraYaw = 0.f, m_CameraPitch = 0.f;
        inline float m_CameraSpeed = 0.1f;
        inline std::string g_ConsoleMSG;
        inline glm::vec3 m_CameraPos = glm::vec3(1.f);
        inline glm::vec3 m_LightPos = glm::vec3(1.f);
        inline glm::vec3 m_LightRot = glm::vec3(0.f);
        inline glm::vec3 m_LightColor = glm::vec3(1.f);
        inline glm::vec3 m_CameraForward = glm::vec3(0.f, 0.f, -1.f);
        inline glm::vec3 m_CameraUp = glm::vec3(0.f, 1.f, 0.f);
		inline GraphicsRenderer* m_GraphicsRender;
		inline ShadowRenderer* m_ShadowRender;
        inline DebugRenderer* m_DbgRender;

        inline static void VK_ASSERT(bool _check)
        {
            if (_check) exit(-69);
        }

        struct GPUInfo
        {
            uint32_t minUniformBufferOffsetAlignment = 0;
            VkPhysicalDevice m_Device = VK_NULL_HANDLE;

            VkPhysicalDeviceProperties			   m_Properties{};
            VkPhysicalDeviceMemoryProperties	   m_MemProps{};
            VkPhysicalDeviceFeatures			   m_Features{};
            std::vector< VkExtensionProperties >   m_ExtensionsProps{};
            std::vector<const char*>               m_DeviceExtensions;
            std::vector< VkDeviceQueueCreateInfo> m_QueueCreateInfos;
            VkSurfaceCapabilitiesKHR		  m_SurfaceCaps{};
            std::vector< VkSurfaceFormatKHR > m_SurfaceFormats{};
            std::vector< VkPresentModeKHR >	  m_PresentModes{};

            std::vector< VkQueueFamilyProperties > m_QueueFamiliesProps{};
            const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
            /*VkPhysicalDeviceProperties2			   properties2{};
            VkPhysicalDeviceMaintenance3Properties properties3{};*/
            VkFormat FindSupportedFormat(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features);

            VkFormat FindDepthTestFormat();
            bool CheckValidationLayerSupport();
            unsigned int FindMemoryType(unsigned int typeFilter, VkMemoryPropertyFlags properties);
            void CheckValidDevice(VkInstance _Instance);
        };

        struct VKContext
        {
            GPUInfo  m_GpuInfo;
            VkDevice m_LogicDevice = VK_NULL_HANDLE;
            
            VkRenderPass m_RenderPass = VK_NULL_HANDLE;
            VkRenderPass m_ShadowPass = VK_NULL_HANDLE;
            unsigned int m_GraphicsQueueFamilyIndex = 0;
            unsigned int m_TransferQueueFamilyIndex = 0;
            VkQueue m_GraphicsQueue;
            VkQueue m_TransferQueue;
            VkQueue m_PresentQueue;

            //std::array< VkPipeline, SWAPCHAIN_BUFFERING_LEVEL > boundGraphicsPipelines{};
            void CreateRenderPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo);
            bool HasStencilComponent(VkFormat format);
            void CreateLogicalDevice();
            void CreateDevice(VkInstance _Instance);
            void CreateShadowRenderPass();
        };
        VKContext& GetVKContext();

        class VKBackend
        {
            //Variables
        public:
        private:
            uint32_t m_LastImageIdx;
            uint32_t m_FrameToPresent;
            bool m_IndexedRender = true;
            bool m_RenderInitialized = false;
            int m_TotalTextures;
            int m_DefualtWidth, m_DefualtHeight, m_DefualtChannels;
            float m_CameraFOV = 70.f;
            GLFWwindow* m_Window;
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
            VkImage m_ShadowImage;
            VkDeviceMemory m_ShadowImageMemory;
            VkImageView m_ShadowImageView;
            R_ShadowMaterial* m_ShadowMat;

            VkSwapchainKHR m_SwapChain;
            // Surface Things
            VkSurfaceFormatKHR m_SurfaceFormat;
            VkSurfaceKHR m_Surface;
            VkExtent2D m_CurrentExtent;
            VkViewport m_Viewport{};
            VkRect2D m_Scissor{};

            VkDescriptorPool m_DescriptorPool;
            VkDescriptorPool m_UIDescriptorPool;
            VkCommandPool m_CommandPool;

            VkBuffer m_StagingBuffer;
            // RENDER BUFFERS
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
            // SHADOW BUFFERS
            std::vector<VkBuffer> m_ShadowUniformBuffers;
            std::vector<VkDeviceMemory> m_ShadowUniformBuffersMemory;
            std::vector<VkBuffer> m_ShadowDynamicBuffers;
            std::vector<VkDeviceMemory> m_ShadowDynamicBuffersMemory;
            std::vector<void*> m_ShadowUniformBuffersMapped;
            std::vector<void*> m_ShadowDynamicBuffersMapped;
            VkDeviceMemory m_StaggingBufferMemory;
            VkPhysicalDeviceMemoryProperties m_Mem_Props;
            VkImage m_DepthImage;
            VkDeviceMemory m_DepthImageMemory;
            VkImageView m_DepthImageView;

            VkClearColorValue defaultClearColor = { { 0.6f, 0.65f, 0.4f, 1.0f } };

            // Para tener mas de un Frame, cada frame debe tener su pack de semaforos y Fencesnot
            VkCommandBuffer m_CommandBuffer[FRAMES_IN_FLIGHT];
            VkSemaphore m_ImageAvailable[FRAMES_IN_FLIGHT];
            VkSemaphore m_RenderFinish[FRAMES_IN_FLIGHT];
            VkFence		m_InFlight[FRAMES_IN_FLIGHT];
            R_Model* tempModel;
            std::vector<R_Model*> m_StaticModels;
            std::vector<R_DbgModel*> m_DbgModels;

            // Functions
        public:
            VKBackend() {}
            VkImageView CreateImageView(VkImage _tImage, VkFormat _format, VkImageAspectFlags _aspectMask);
            void CreateImageViews();
            void Init();
        private:
            void InitializeVulkan(VkApplicationInfo* _appInfo);
            void CreateInstance(VkInstanceCreateInfo* _createInfo, VkApplicationInfo* _appInfo,
                                const char** m_Extensions,
                                uint32_t m_extensionCount);
            void CreateSwapChain();
            void RecreateSwapChain();
            void CreateShadowFramebuffer();
            void CreateFramebuffers(Renderer* _renderer);
            void CreateCommandBuffer();
            void CreateSyncObjects(unsigned _frameIdx);
            void CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkSharingMode _shareMode,
                              VkMemoryPropertyFlags _memFlags, VkBuffer& buffer_, VkDeviceMemory& bufferMem_);
            void CreateAndTransitionImage(char* _modelPath, Texture* _texture);
            void CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _w, uint32_t _h,
                                   VkDeviceSize _bufferOffset);
            void CreateImage(unsigned _Width, unsigned _Height, VkFormat _format, VkImageTiling _tiling,
                             VkImageUsageFlagBits _usage, VkMemoryPropertyFlags _memProperties, VkImage* _image,
                             VkDeviceMemory* _imageMem);
            void CreateShadowResources();
            void CreateDepthTestingResources();
            VkImageView CreateTextureImageView(VkImage _tImage);
            VkSampler CreateTextureSampler();
            VkCommandBuffer BeginSingleTimeCommandBuffer();
            void EndSingleTimeCommandBuffer(VkCommandBuffer _commandBuffer);
            void CopyBuffer(VkBuffer dst_, VkBuffer _src, VkDeviceSize _size);
            void TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _old, VkImageLayout _new);
            void RecordCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIdx, unsigned int _frameIdx,
                                     Renderer* _renderer, ImDrawData* draw_data = nullptr);
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
            void ProcessModelNode(aiNode* _node, const aiScene* _scene);

        public:
            void LoadModel(const char* _filepath, const char* _modelName);
            bool BackendShouldClose();
            void PollEvents();
            void EditorLoop();
            void DrawFrame(unsigned int _InFlightFrame);
            void SubmitAndPresent(unsigned _FrameToPresent, uint32_t* _imageIdx);
            void Cleanup();
        };
        VKBackend& GetVKBackend();
    }
}

