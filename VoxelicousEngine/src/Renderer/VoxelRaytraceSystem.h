#pragma once

#include "Device.h"
#include "Camera.h"
#include "../Voxel/VoxelWorld.h"
#include "../Core/Window.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Buffer.h"

namespace VoxelicousEngine {

    // Forward declarations
    class Texture;

    class VoxelRaytraceSystem {
    public:
        VoxelRaytraceSystem(Device& device, const Window& window, VoxelWorld& world, Camera& camera);
        ~VoxelRaytraceSystem();

        VoxelRaytraceSystem(const VoxelRaytraceSystem&) = delete;
        VoxelRaytraceSystem& operator=(const VoxelRaytraceSystem&) = delete;

        // Performs the CPU ray tracing for the current frame
        void TraceRays();

        // Updates the Vulkan texture with the latest ray tracing results
        // Needs command buffer to perform copy operation
        void UpdateOutputTexture(VkCommandBuffer commandBuffer);

        // Get the output texture for rendering
        const Texture& GetOutputTexture() const;
        VkDescriptorSet GetOutputTextureDescriptorSet() const; // For ImGui or full-screen quad
        VkDescriptorSetLayout GetOutputTextureDescriptorSetLayout() const; // Added getter for layout

        // Handle window resize
        void OnResize(uint32_t width, uint32_t height);

        void OnRender(VkCommandBuffer commandBuffer);
        void OnEvent(Event& event);
        void UpdateGpuResources(VkCommandBuffer commandBuffer);
        void DispatchCompute(VkCommandBuffer commandBuffer, uint32_t frameIndex);

    private:
        void CreateOutputTexture(uint32_t width, uint32_t height);
        void CreateCpuPixelBuffer(uint32_t width, uint32_t height);
        void CreateDescriptorSet(); // For the output texture (display pipeline)
        glm::vec4 ShadeHit(const SparseVoxelOctree::IntersectionResult& hit, const Ray& ray) const; // Simple shading

        // --- Compute Helpers ---
        void CreateComputeResources();
        void CreateOrResizeSvoBuffers(); // Creates/Resizes GPU buffers for SVO data
        void UpdateSvoBuffers(); // Uploads current SVO data to GPU buffers
        void CreateComputeUboBuffers();
        void UpdateComputeUboBuffer(uint32_t frameIndex);
        void CreateComputePipeline();
        void CreateComputeDescriptorSets();

        Device& m_Device;
        const Window& m_Window; // To get dimensions
        VoxelWorld& m_World;
        Camera& m_Camera;

        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        
        // CPU-side buffer for pixel data (RGBA float)
        std::vector<glm::vec4> m_CpuPixelBuffer;
        
        // Vulkan resources for the output texture
        std::unique_ptr<Texture> m_OutputTexture;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE; // Pool specific to this system's descriptor set(s)
        VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE; // Layout for the output texture
        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE; // Descriptor set referencing the output texture

        // --- Compute Shader Resources ---
        std::unique_ptr<Buffer> m_SvoNodeBuffer;       // Holds OctreeNode pool
        std::unique_ptr<Buffer> m_VoxelPaletteBuffer;  // Holds Voxel definitions
        std::unique_ptr<Buffer> m_MaterialPaletteBuffer; // Holds VoxelMaterial definitions
        std::vector<std::unique_ptr<Buffer>> m_ComputeUboBuffers; // Uniforms for compute shader (per frame)

        VkPipelineLayout m_ComputePipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_ComputePipeline = VK_NULL_HANDLE;
        VkDescriptorPool m_ComputeDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_ComputeSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_ComputeDescriptorSets; // Per frame

        bool m_NeedsResize = false;
    };

} // namespace VoxelicousEngine 