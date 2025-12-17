#pragma once
#include "../core/VKRRenderPass.h"
#include <string.h>
#include <string>

namespace VKR
{
    namespace render
    {
        extern std::string g_ConsoleMSG;
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
             *		VkPhysicalDeviceMaintenance3Properties properties3{};*/
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

            VKR::render::RenderPass* m_RenderPass;
            VKR::render::RenderPass* m_ShadowPass;

            unsigned int m_GraphicsComputeQueueFamilyIndex = 0;
            unsigned int m_TransferQueueFamilyIndex = 0;
            VkQueue m_GraphicsComputeQueue;
            VkQueue m_TransferQueue;
            VkQueue m_PresentQueue;

            //std::array< VkPipeline, SWAPCHAIN_BUFFERING_LEVEL > boundGraphicsPipelines{};
            void CreateRenderPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo);
            void CreateGeometryPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo);
            bool HasStencilComponent(VkFormat format);
            void CreateLogicalDevice();
            void CreateDevice(VkInstance _Instance);
            void CreateShadowRenderPass();
            void Cleanup();
        };
        inline VKContext g_context;
        inline VKContext& GetVKContext()
        {
            return g_context;
        }
    }
}
