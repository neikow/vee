# Roadmap for Renderer Improvements

## Phase 1: Resource Management & Safety

Goal: Stop the renderer from crashing during resource deletion and handle memory more efficiently.

- [x] Implement a Frame-Delayed Deletion Queue
    - [x] Create a struct ResourceDeletion containing the Vulkan handle and the frameIndex it was deleted on.
    - [x] Only call vkDestroy... when currentFrame > deletionFrame + MAX_FRAMES_IN_FLIGHT.

- [x] Abstract Vulkan Memory Allocator (VMA)
    - [x] Integrate the AMD Vulkan Memory Allocator library.
    - [x] Replace manual vkAllocateMemory and vkBindBufferMemory calls with vmaCreateBuffer.
    - [x] Add memory usage in Editor UI.

- [x] Unified Geometry Buffer (The "Big Buffer")
    - [x] Allocate one large VkBuffer for all Vertices and one for all Indices.
    - [x] Update MeshManager to return offset and size instead of creating new buffers per mesh.

## Phase 2: Decoupling & Modularization

Goal: Break the monolithic Renderer class into smaller, specialized units.

- [x] Create a VulkanDevice Class
    - [x] Move Instance, Physical Device, Logical Device, and Queue management here.

- [x] Create a Shader & Pipeline Cache
    - [x] Implement a system that loads SPIR-V from files.
    - [x] Create a hash for Pipeline states so you don't recreate identical pipelines.
    - [x] Encapsulate the Swapchain
    - [x] Move VkSwapchainKHR, ImageViews, and Framebuffers into a VulkanSwapchain class.
    - [x] Simplify the RecreateSwapChain logic to be a single method call.

## Phase 3: The "Bindless" Revolution

Goal: Remove the 2048 texture limit and stop updating Descriptor Sets every frame.

- [x] Implement Bindless Textures
    - [x] Update your DescriptorSetLayout to use VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT.
    - [x] Increase your texture array size to the physical device limit (often 100k+).
    - [x] Pass the textureId solely through Push Constants; stop calling vkUpdateDescriptorSets per texture load.
- [x] Global UBO Management
    - [x] Use a single "Global Data" UBO for View/Projection matrices.
    - [x] Use "Dynamic Offsets" or a large buffer to store per-object data if it exceeds Push Constant limits.

## Phase 4: Render Graph & Multi-Pass

Goal: Formalize how different passes (Shadows, Picking, UI, Main) talk to each other.

- [ ] Define a RenderPass Interface
    - [ ] Create a base class where each pass defines its own Execute(VkCommandBuffer) method.
- [ ] Implement Synchronization Automated Logic
    - [ ] Create a system that automatically inserts VkImageMemoryBarrier based on which pass reads/writes to which
      texture.
- [ ] Async Picking
    - [ ] Move the GetEntityIDAt logic to be non-blocking.
    - [ ] Render the picking ID buffer in the background and read the result from the previous frame to avoid
      vkDeviceWaitIdle.

## Phase 5: Advanced Features & Optimization

Goal: Polish and performance.

- [ ] Add an Instance System
    - [ ] Detect when the same meshId is drawn multiple times and use vkCmdDrawIndexedInstanced.
- [ ] Frustum Culling
    - [ ] Use the Octree you built earlier to only submit SubmitDrawCall for entities inside the camera frustum.
- [ ] Command Buffer Recording Optimization
    - [ ] Sort the m_DrawQueue by Pipeline and then by Material to minimize state changes.