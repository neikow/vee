#include "vulkan_device.h"

#include "consts.h"
#include "utils.h"
#include "../../engine.h"

Vulkan::VulkanDevice::VulkanDevice(const std::shared_ptr<Window> &window) : m_Window(window) {
}

VkDevice &Vulkan::VulkanDevice::GetLogicalDevice() {
    return m_Device;
}

VkPhysicalDevice &Vulkan::VulkanDevice::GetPhysicalDevice() {
    return m_PhysicalDevice;
}

VkInstance &Vulkan::VulkanDevice::GetInstance() {
    return m_Instance;
}

VkQueue &Vulkan::VulkanDevice::GetGraphicsQueue() {
    return m_GraphicsQueue;
}

VkQueue &Vulkan::VulkanDevice::GetPresentQueue() {
    return m_PresentQueue;
}

VkQueue &Vulkan::VulkanDevice::GetTransferQueue() {
    return m_TransferQueue;
}

VmaAllocator &Vulkan::VulkanDevice::GetAllocator() {
    return m_Allocator;
}

VkSurfaceKHR &Vulkan::VulkanDevice::GetSurface() {
    return m_Surface;
}

void Vulkan::VulkanDevice::Initialize(const std::string &appName, const uint32_t version) {
    CreateInstance(appName, version);
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateCommandPools();
}

void Vulkan::VulkanDevice::WaitIdle() const {
    vkDeviceWaitIdle(m_Device);
}

void Vulkan::VulkanDevice::CreateSurface() {
    if (
        glfwCreateWindowSurface(
            m_Instance,
            m_Window->GetWindow(),
            nullptr,
            &m_Surface
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Vulkan::VulkanDevice::CreateLogicalDevice() {
    const auto indices = FindQueueFamilies(
        m_PhysicalDevice
    );

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value(),
        indices.transferFamily.value(),
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
    };
    features12.bufferDeviceAddress = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    createInfo.pNext = &features12;

    if constexpr (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(ACTIVE_VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = ACTIVE_VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
    vkGetDeviceQueue(m_Device, indices.transferFamily.value(), 0, &m_TransferQueue);

    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = m_PhysicalDevice,
        .device = m_Device,
        .pVulkanFunctions = &vulkanFunctions,
        .instance = m_Instance,
        .vulkanApiVersion = VK_API_VERSION_1_4,
    };

    if (vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator) != VK_SUCCESS) {
        throw std::runtime_error("failed to create VMA allocator!");
    }
}

void Vulkan::VulkanDevice::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(
        m_Instance,
        &deviceCount,
        nullptr
    );

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto &physicalDevice: devices) {
        int score = RateDeviceSuitability(physicalDevice);
        candidates.insert(std::make_pair(score, physicalDevice));
    }

    if (candidates.rbegin()->first > 0) {
        m_PhysicalDevice = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void Vulkan::VulkanDevice::SetupDebugMessenger() {
    if constexpr (!ENABLE_VALIDATION_LAYERS) return;

    const auto createInfo = Utils::GetDebugMessageCreateInfo(
        SEVERITY,
        MESSAGE_TYPE
    );

    if (
        Utils::CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) !=
        VK_SUCCESS
    ) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void Vulkan::VulkanDevice::CreateInstance(const std::string &appName, const uint32_t version) {
    if constexpr (ENABLE_VALIDATION_LAYERS) {
        if (!Utils::CheckValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = version;
    appInfo.pEngineName = ENGINE_NAME;
    appInfo.engineVersion = ENGINE_VERSION;
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    const auto extensions = Utils::GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    if constexpr (ENABLE_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(ACTIVE_VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = ACTIVE_VALIDATION_LAYERS.data();

        const auto debugCreateInfo = Utils::GetDebugMessageCreateInfo(
            SEVERITY,
            MESSAGE_TYPE
        );

        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }


    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

int Vulkan::VulkanDevice::RateDeviceSuitability(const VkPhysicalDevice &physicalDevice) const {
    VkPhysicalDeviceProperties deviceProperties;

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    deviceFeatures2.pNext = &indexingFeatures;

    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

    int score = 0;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 100;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    auto indices = FindQueueFamilies(physicalDevice);

    if (!indices.isComplete()) {
        return 0;
    }

    bool extensionsSupported = Utils::CheckPhysicalDeviceExtensionSupport(physicalDevice);
    if (!extensionsSupported) {
        return 0;
    }

    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);
    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
        return 0;
    }

    if (
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing != VK_TRUE ||
        indexingFeatures.runtimeDescriptorArray != VK_TRUE ||
        indexingFeatures.descriptorBindingPartiallyBound != VK_TRUE ||
        indexingFeatures.descriptorBindingVariableDescriptorCount != VK_TRUE ||
        indexingFeatures.descriptorBindingSampledImageUpdateAfterBind != VK_TRUE
    ) {
        return 0;
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
    if (!supportedFeatures.samplerAnisotropy) {
        return 0;
    }

    return score;
}

Vulkan::QueueFamilyIndices Vulkan::VulkanDevice::FindQueueFamilies(const VkPhysicalDevice &physicalDevice) const {
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily: queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (
            queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT
            && queueFamily.queueFlags & ~VK_QUEUE_GRAPHICS_BIT
        ) {
            indices.transferFamily = i;
        }


        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void Vulkan::VulkanDevice::CreateCommandPools() {
    const QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(
        m_PhysicalDevice
    );

    VkCommandPoolCreateInfo graphicsPoolInfo{};
    graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    graphicsPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (
        vkCreateCommandPool(
            m_Device,
            &graphicsPoolInfo,
            nullptr,
            &m_GraphicsCommandPool
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("failed to create command pool!");
    }

    VkCommandPoolCreateInfo transferPoolInfo{};
    transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transferPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    transferPoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

    if (
        vkCreateCommandPool(
            m_Device,
            &transferPoolInfo,
            nullptr,
            &m_TransferCommandPool
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("failed to create command pool!");
    }
}

Vulkan::SwapChainSupportDetails Vulkan::VulkanDevice::QuerySwapChainSupport(const VkPhysicalDevice &device) const {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            m_Surface,
            &presentModeCount,
            details.presentModes.data()
        );
    }

    return details;
}

void Vulkan::VulkanDevice::Cleanup() const {
    vmaDestroyAllocator(m_Allocator);

    vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
    vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);

    vkDestroyDevice(m_Device, nullptr);

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    Utils::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void Vulkan::VulkanDevice::DestroyImageView(const VkImageView &imageView) const {
    vkDestroyImageView(m_Device, imageView, nullptr);
}

void Vulkan::VulkanDevice::DestroyImage(const VkImage &image, const VmaAllocation &allocation) const {
    vmaDestroyImage(m_Allocator, image, allocation);
}

void Vulkan::VulkanDevice::MapMemory(
    const VmaAllocation &allocation,
    void **data
) const {
    vmaMapMemory(m_Allocator, allocation, data);
}

void Vulkan::VulkanDevice::UnmapMemory(
    const VmaAllocation &allocation
) const {
    vmaUnmapMemory(m_Allocator, allocation);
}

void Vulkan::VulkanDevice::DestroyBuffer(const VkBuffer &buffer, const VmaAllocation &allocation) const {
    vmaDestroyBuffer(m_Allocator, buffer, allocation);
}

VkCommandPool Vulkan::VulkanDevice::GetTransferCommandPool() const {
    return m_TransferCommandPool;
}

VkCommandPool Vulkan::VulkanDevice::GetGraphicsCommandPool() const {
    return m_GraphicsCommandPool;
}

VmaAllocationInfo Vulkan::VulkanDevice::GetAllocationInfo(const VmaAllocation &allocation) const {
    VmaAllocationInfo info;
    vmaGetAllocationInfo(m_Allocator, allocation, &info);
    return info;
}

VkFramebuffer Vulkan::VulkanDevice::CreateFramebuffer(
    const VkFramebufferCreateInfo &framebufferCreateInfo
) const {
    VkFramebuffer framebuffer;
    if (
        vkCreateFramebuffer(
            m_Device,
            &framebufferCreateInfo,
            nullptr,
            &framebuffer
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create framebuffer.");
    }
    return framebuffer;
}

VkRenderPass Vulkan::VulkanDevice::CreateRenderPass(
    const VkRenderPassCreateInfo &renderPassCreateInfo
) const {
    VkRenderPass renderPass;
    if (
        vkCreateRenderPass(
            m_Device,
            &renderPassCreateInfo,
            nullptr,
            &renderPass
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create render pass.");
    }
    return renderPass;
}

VkPipeline Vulkan::VulkanDevice::CreatePipeline(
    const VkGraphicsPipelineCreateInfo &pipelineCreateInfo
) const {
    VkPipeline pipeline;
    if (
        vkCreateGraphicsPipelines(
            m_Device,
            VK_NULL_HANDLE,
            1,
            &pipelineCreateInfo,
            nullptr,
            &pipeline
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create graphics pipeline.");
    }
    return pipeline;
}

void Vulkan::VulkanDevice::DestroyFramebuffer(const VkFramebuffer &framebuffer) const {
    vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
}

void Vulkan::VulkanDevice::DestroyShaderModule(const VkShaderModule &shaderModule) const {
    vkDestroyShaderModule(m_Device, shaderModule, nullptr);
}

VkDescriptorPool Vulkan::VulkanDevice::CreateDescriptorPool(
    const VkDescriptorPoolCreateInfo &descriptorPoolCreateInfo
) const {
    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(
            m_Device,
            &descriptorPoolCreateInfo,
            nullptr,
            &descriptorPool
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create descriptor pool.");
    }
    return descriptorPool;
}

void Vulkan::VulkanDevice::DestroyDescriptorPool(const VkDescriptorPool &descriptorPool) const {
    vkDestroyDescriptorPool(m_Device, descriptorPool, nullptr);
}

void Vulkan::VulkanDevice::DestroyRenderPass(const VkRenderPass &renderPass) const {
    vkDestroyRenderPass(m_Device, renderPass, nullptr);
}

void Vulkan::VulkanDevice::DestroyPipeline(const VkPipeline &pipeline) const {
    vkDestroyPipeline(m_Device, pipeline, nullptr);
}

void Vulkan::VulkanDevice::DestroySemaphore(const VkSemaphore &semaphore) const {
    vkDestroySemaphore(m_Device, semaphore, nullptr);
}

void Vulkan::VulkanDevice::DestroyFence(const VkFence &fence) const {
    vkDestroyFence(m_Device, fence, nullptr);
}

void Vulkan::VulkanDevice::DestroyPipelineLayout(const VkPipelineLayout &pipelineLayout) const {
    vkDestroyPipelineLayout(m_Device, pipelineLayout, nullptr);
}

void Vulkan::VulkanDevice::DestroyDescriptorSetLayout(const VkDescriptorSetLayout &descriptorSetLayout) const {
    vkDestroyDescriptorSetLayout(m_Device, descriptorSetLayout, nullptr);
}

void Vulkan::VulkanDevice::DestroySampler(const VkSampler &sampler) const {
    vkDestroySampler(m_Device, sampler, nullptr);
}

void Vulkan::VulkanDevice::DestroySwapchain(const VkSwapchainKHR &swapchain) const {
    vkDestroySwapchainKHR(m_Device, swapchain, nullptr);
}

VkShaderModule Vulkan::VulkanDevice::CreateShaderModule(const VkShaderModuleCreateInfo &shaderModuleCreateInfo) const {
    VkShaderModule shaderModule;
    if (
        vkCreateShaderModule(
            m_Device,
            &shaderModuleCreateInfo,
            nullptr,
            &shaderModule
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create shader module.");
    }
    return shaderModule;
}

VkPipelineLayout Vulkan::VulkanDevice::CreatePipelineLayout(
    const VkPipelineLayoutCreateInfo &pipeline_layout_info) const {
    VkPipelineLayout pipelineLayout;
    if (
        vkCreatePipelineLayout(
            m_Device,
            &pipeline_layout_info,
            nullptr,
            &pipelineLayout
        ) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create pipeline layout.");
    }

    return pipelineLayout;
}
