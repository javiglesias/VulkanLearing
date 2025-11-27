#include "VKDevice.h"


namespace VKR {
    namespace render {
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
            render::g_ConsoleMSG += "No found supported format\n";
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
                fprintf(stderr, "NOT FOUND layer: %s", layerName);
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
            for (unsigned int it = 0; it < deviceCount; it++)
            {
                vkGetPhysicalDeviceProperties(m_PhysicalDevices[it], &deviceProp[it]);
                if (strstr(deviceProp[it].deviceName, "NVIDIA") || strstr(deviceProp[it].deviceName, "AMD"))
                {
                    m_GPUSelected = it;
                    char tmp[512];
                    sprintf(tmp, "\nGPU %d: %s\n", it, deviceProp[m_GPUSelected].deviceName);
                    render::g_ConsoleMSG += tmp;
                    g_context.m_GpuInfo.minUniformBufferOffsetAlignment = (uint32_t)deviceProp[m_GPUSelected].limits.minUniformBufferOffsetAlignment;
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
            m_CreateInfo.queueCreateInfoCount = (uint32_t)g_context.m_GpuInfo.m_QueueCreateInfos.size();
            m_CreateInfo.pEnabledFeatures = &m_GpuInfo.m_Features;
            m_CreateInfo.enabledExtensionCount = (uint32_t)g_context.m_GpuInfo.m_DeviceExtensions.size();
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
            bool searchingGraphicsCompute = true, searchingTransfer = true;
            for (const auto& queueFamily : queueFamilies)
            {
                if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    searchingGraphicsCompute = false;
                }

                if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                {
                    searchingTransfer = false;
                }

                m_GraphicsComputeQueueFamilyIndex += searchingGraphicsCompute;
                m_TransferQueueFamilyIndex += searchingTransfer;
                if (!searchingGraphicsCompute && !searchingTransfer)
                    break;
            }
            char tmp[256];
            sprintf(tmp, "Graphics Family: %d\n", m_GraphicsComputeQueueFamilyIndex);
            sprintf(tmp, "Transfer Family: %d\n", m_TransferQueueFamilyIndex);
            render::g_ConsoleMSG += tmp;
            /// Ahora vamos a crear el device logico para interactuar con �l
            float queuePriority = 1.f;

            g_context.m_GpuInfo.m_QueueCreateInfos.push_back({
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m_GraphicsComputeQueueFamilyIndex,
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
            vkGetDeviceQueue(m_LogicDevice, m_GraphicsComputeQueueFamilyIndex, 0, &m_GraphicsComputeQueue);
            vkGetDeviceQueue(m_LogicDevice, m_TransferQueueFamilyIndex, 0, &m_TransferQueue);
        }

        void VKContext::CreateShadowRenderPass()
        {
            m_ShadowPass = new VKR::render::RenderPass();
            m_ShadowPass->CreateDepthAttachment(m_GpuInfo.FindDepthTestFormat(),
                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            m_ShadowPass->CreateDepthOnlySubPass();
            m_ShadowPass->CreatePass(m_LogicDevice);
        }

        void VKContext::Cleanup()
        {
            vkDeviceWaitIdle(m_LogicDevice);
            m_ShadowPass->Cleanup(m_LogicDevice);
            m_RenderPass->Cleanup(m_LogicDevice);
        }

        void VKContext::CreateRenderPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo)
        {

            m_RenderPass = new VKR::render::RenderPass();
            m_RenderPass->CreateColorAttachment(m_SwapChainCreateInfo->imageFormat);
            m_RenderPass->CreateDepthAttachment(m_GpuInfo.FindDepthTestFormat(),
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            m_RenderPass->CreateSubPass();
            m_RenderPass->CreatePass(m_LogicDevice);
        }

        void VKContext::CreateGeometryPass(VkSwapchainCreateInfoKHR* m_SwapChainCreateInfo)
        {
            #if 0

            m_GeometryPass = new VKR::render::RenderPass();
            m_GeometryPass->CreateGeometryAttachment(m_SwapChainCreateInfo->imageFormat);
            m_GeometryPass->CreateDepthAttachment(m_GpuInfo.FindDepthTestFormat(),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            m_GeometryPass->CreateSubPass();
            m_GeometryPass->CreatePass(m_LogicDevice);
            #endif
        }

        bool VKContext::HasStencilComponent(VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }
    }
}
