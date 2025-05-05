#include "vepch.h"
#include "VoxelRaytraceSystem.h"
#include "Texture.h"       // The texture class we just created
#include "Buffer.h"        // Assuming this exists for descriptor writing
#include "Descriptors.h"
#include "Core/Log.h"
#include "../Voxel/SparseVoxelOctree.h" // Need Ray definition
#include "Pipeline.h" // Added include
#include "ShaderManager.h" // Added include
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept> // For std::runtime_error
#include <chrono>    // For timing
#include <future>    // For std::async (optional multi-threading)

// TODO: Remove later or make configurable
#define ENABLE_MULTITHREADED_CPU_TRACE 0 // Disabled CPU trace

namespace VoxelicousEngine {

    // Define the structure for compute shader uniforms
    struct ComputeUbo {
        glm::mat4 InverseView{1.f};
        glm::mat4 InverseProjection{1.f};
        glm::vec4 CameraPosition{0.f};
        glm::vec2 Resolution{0.f}; // Store width, height
        // Use vec4 for std140 alignment
        glm::vec3 OctreeCenter{0.f}; // Pad w component
        float OctreeSize = 0.f; 
        // Add padding here if necessary, but vec4 followed by float is usually fine
    };

    // --- Constructor & Destructor ---

    VoxelRaytraceSystem::VoxelRaytraceSystem(Device& device, const Window& window, VoxelWorld& world, Camera& camera)
        : m_Device(device), m_Window(window), m_World(world), m_Camera(camera)
    {
        m_Width = window.GetWidth();
        m_Height = window.GetHeight();
        VE_CORE_ASSERT(m_Width > 0 && m_Height > 0, "Window dimensions must be valid");

        // Removed CPU Pixel Buffer Creation
        // CreateCpuPixelBuffer(m_Width, m_Height);
        CreateOutputTexture(m_Width, m_Height); // Creates texture with storage usage
        CreateDescriptorSet(); // Creates descriptors for display pipeline
        
        // Create GPU Buffers for SVO data
        CreateOrResizeSvoBuffers();
        UpdateSvoBuffers(); // Initial upload
        
        // Create Compute UBOs, Pipeline, Descriptors
        CreateComputeResources(); // Call the main setup function
    }

    VoxelRaytraceSystem::~VoxelRaytraceSystem() {
        // Destroy display descriptor pool/layout
        if (m_DescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, nullptr);
        }
        if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_DescriptorSetLayout, nullptr);
        }
        
        // Destroy compute pipeline and layout
        if(m_ComputePipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_Device.GetDevice(), m_ComputePipeline, nullptr);
        }
        if(m_ComputePipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_Device.GetDevice(), m_ComputePipelineLayout, nullptr);
        }
        // Destroy compute descriptor pool/layout
        if(m_ComputeDescriptorPool != VK_NULL_HANDLE) {
             vkDestroyDescriptorPool(m_Device.GetDevice(), m_ComputeDescriptorPool, nullptr);
        }
         if(m_ComputeSetLayout != VK_NULL_HANDLE) {
             vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_ComputeSetLayout, nullptr);
        }

        // unique_ptrs handle m_OutputTexture and buffer destruction
    }

    // --- Resource Creation / Management ---

    void VoxelRaytraceSystem::CreateCpuPixelBuffer(uint32_t width, uint32_t height) {
        m_CpuPixelBuffer.resize(width * height);
        m_Width = width;
        m_Height = height;
    }

    void VoxelRaytraceSystem::CreateOutputTexture(uint32_t width, uint32_t height) {
        // Use a common format like RGBA8 or RGBA32F
        // RGBA32F is better for precision but uses more memory/bandwidth
        VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;

        // Usage flags required: Transfer Dst (upload), Sampled (read), Storage (compute write)
        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

        m_OutputTexture = std::make_unique<Texture>(m_Device, width, height, format, usage);
        
        // Initial transition to Shader Read Only, as it will be sampled
        VkCommandBuffer cmdbuf = m_Device.BeginSingleTimeCommands();
        m_OutputTexture->TransitionLayout(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_Device.EndSingleTimeCommands(cmdbuf);
        
        m_NeedsResize = false; // Reset resize flag
    }

    void VoxelRaytraceSystem::CreateDescriptorSet() {
        // --- Create Descriptor Pool ---
        // Pool size just for one descriptor set with one combined image sampler
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Allow freeing if needed

        if (vkCreateDescriptorPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to create descriptor pool for VoxelRaytraceSystem!");
            throw std::runtime_error("Failed to create descriptor pool!");
        }

        // --- Create Descriptor Set Layout ---
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = 0; // Binding point in shader
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Used in fragment shader (full-screen quad)
        layoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &layoutBinding;

        if (vkCreateDescriptorSetLayout(m_Device.GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
             VE_CORE_FATAL("Failed to create descriptor set layout for VoxelRaytraceSystem!");
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        // --- Allocate Descriptor Set ---
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_DescriptorSetLayout;

        if (vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to allocate descriptor set for VoxelRaytraceSystem!");
            throw std::runtime_error("Failed to allocate descriptor set!");
        }

        // --- Update Descriptor Set ---
        // Point the descriptor set to our output texture
        VkDescriptorImageInfo imageInfo = m_OutputTexture->GetDescriptorInfo(); 
        
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_Device.GetDevice(), 1, &descriptorWrite, 0, nullptr);
    }

    // --- Compute Shader Resource Helpers ---

    void VoxelRaytraceSystem::CreateOrResizeSvoBuffers() {
        const auto& octree = m_World.GetOctree();
        const auto& nodePool = octree.GetNodePool();
        const auto& voxelPalette = octree.GetVoxelPalette(); // Use getter
        const auto& materialPalette = m_World.GetMaterialPalette();

        VkDeviceSize nodeBufferSize = sizeof(OctreeNode) * nodePool.size();
        VkDeviceSize voxelPalBufferSize = sizeof(Voxel) * voxelPalette.size();
        VkDeviceSize materialPalBufferSize = sizeof(VoxelMaterial) * materialPalette.size();
        
        bool recreateNodeBuffer = !m_SvoNodeBuffer || m_SvoNodeBuffer->GetBufferSize() < nodeBufferSize;
        bool recreateVoxelPalBuffer = !m_VoxelPaletteBuffer || m_VoxelPaletteBuffer->GetBufferSize() < voxelPalBufferSize;
        bool recreateMaterialPalBuffer = !m_MaterialPaletteBuffer || m_MaterialPaletteBuffer->GetBufferSize() < materialPalBufferSize;

        // If buffers need resizing, recreate them. We assume the Buffer class handles cleanup of old buffer.
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (recreateNodeBuffer && nodeBufferSize > 0) {
            VE_CORE_INFO("Recreating SVO Node Buffer (size: {})", nodeBufferSize);
            m_SvoNodeBuffer = std::make_unique<Buffer>(m_Device, nodeBufferSize, 1, usage, memProps);
        }
        if (recreateVoxelPalBuffer && voxelPalBufferSize > 0) {
             VE_CORE_INFO("Recreating Voxel Palette Buffer (size: {})", voxelPalBufferSize);
            m_VoxelPaletteBuffer = std::make_unique<Buffer>(m_Device, voxelPalBufferSize, 1, usage, memProps);
        }
        if (recreateMaterialPalBuffer && materialPalBufferSize > 0) {
             VE_CORE_INFO("Recreating Material Palette Buffer (size: {})", materialPalBufferSize);
            m_MaterialPaletteBuffer = std::make_unique<Buffer>(m_Device, materialPalBufferSize, 1, usage, memProps);
        }
        
        // TODO: Need to handle update of descriptor sets if buffers are recreated.
    }
    
    // Uploads current SVO data to existing GPU buffers using staging buffer mechanism
    void VoxelRaytraceSystem::UpdateSvoBuffers() {
        const auto& octree = m_World.GetOctree();
        const auto& nodePool = octree.GetNodePool();
        const auto& voxelPalette = octree.GetVoxelPalette(); // Use getter
        const auto& materialPalette = m_World.GetMaterialPalette();
        
        VkDeviceSize nodeDataSize = sizeof(OctreeNode) * nodePool.size();
        VkDeviceSize voxelPalDataSize = sizeof(Voxel) * voxelPalette.size();
        VkDeviceSize materialPalDataSize = sizeof(VoxelMaterial) * materialPalette.size();

        // Use UploadDataViaStaging for device-local buffers
        if (m_SvoNodeBuffer && nodeDataSize > 0) {
            if (nodeDataSize <= m_SvoNodeBuffer->GetBufferSize()) {
                m_SvoNodeBuffer->UploadDataViaStaging(nodePool.data(), nodeDataSize);
            } else {
                 VE_CORE_WARN("SVO Node Buffer too small to update. Data size: {}, Buffer size: {}", nodeDataSize, m_SvoNodeBuffer->GetBufferSize());
                 // Consider resizing here with CreateOrResizeSvoBuffers()?
            }
        } else if (nodeDataSize > 0) {
             VE_CORE_WARN("SVO Node Buffer invalid for update.");
        }
        
        if (m_VoxelPaletteBuffer && voxelPalDataSize > 0) {
            if (voxelPalDataSize <= m_VoxelPaletteBuffer->GetBufferSize()) {
                 m_VoxelPaletteBuffer->UploadDataViaStaging(voxelPalette.data(), voxelPalDataSize);
            } else {
                 VE_CORE_WARN("Voxel Palette Buffer too small to update.");
            }
        } else if (voxelPalDataSize > 0) {
             VE_CORE_WARN("Voxel Palette Buffer invalid for update.");
        }
        
        if (m_MaterialPaletteBuffer && materialPalDataSize > 0) {
            if (materialPalDataSize <= m_MaterialPaletteBuffer->GetBufferSize()) {
                 m_MaterialPaletteBuffer->UploadDataViaStaging(materialPalette.data(), materialPalDataSize);
            } else {
                 VE_CORE_WARN("Material Palette Buffer too small to update.");
            }
        } else if (materialPalDataSize > 0) {
             VE_CORE_WARN("Material Palette Buffer invalid for update.");
        }
        
        // Synchronization might still be needed depending on Buffer implementation
    }

    void VoxelRaytraceSystem::CreateComputeUboBuffers() {
        m_ComputeUboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < m_ComputeUboBuffers.size(); i++) {
            m_ComputeUboBuffers[i] = std::make_unique<Buffer>(
                m_Device,
                sizeof(ComputeUbo),
                1, // instance count
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // Host visible for easy update
            );
            m_ComputeUboBuffers[i]->Map(); // Keep mapped for frequent updates
        }
    }

    void VoxelRaytraceSystem::UpdateComputeUboBuffer(uint32_t frameIndex) {
         if (frameIndex >= m_ComputeUboBuffers.size() || !m_ComputeUboBuffers[frameIndex]) {
             VE_CORE_ERROR("Invalid frame index or UBO buffer not created for compute shader!");
             return;
         }

        ComputeUbo ubo{};
        ubo.InverseView = glm::inverse(m_Camera.GetView());
        ubo.InverseProjection = glm::inverse(m_Camera.GetProjection());
        ubo.CameraPosition = glm::vec4(m_Camera.GetPosition(), 1.0f); // w=1 for vec4 alignment/convenience
        ubo.Resolution = { (float)m_Width, (float)m_Height };
        // Add octree parameters
        const auto& octree = m_World.GetOctree();
        ubo.OctreeCenter = glm::vec4(octree.GetCenter(), 0.0f); 
        ubo.OctreeSize = octree.GetSize();

        m_ComputeUboBuffers[frameIndex]->WriteToBuffer(&ubo); 
        m_ComputeUboBuffers[frameIndex]->Flush(); // Ensure data is flushed if not using coherent memory
    }

    void VoxelRaytraceSystem::CreateComputeDescriptorSets() {
        // Create Pool
        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT },        // Compute UBO
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SwapChain::MAX_FRAMES_IN_FLIGHT },         // Output Image
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT * 3 }     // Node Pool, Voxel Palette, Material Palette (assuming updated per frame for simplicity for now)
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = SwapChain::MAX_FRAMES_IN_FLIGHT;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        if (vkCreateDescriptorPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_ComputeDescriptorPool) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to create compute descriptor pool!");
        }

        // Create Layout
         std::vector<VkDescriptorSetLayoutBinding> bindings = {
            // Binding 0: Compute UBO
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            // Binding 1: Output Texture
            {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            // Binding 2: SVO Node Buffer
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
             // Binding 3: Voxel Palette Buffer
            {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
             // Binding 4: Material Palette Buffer
            {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}
        };

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_Device.GetDevice(), &layoutInfo, nullptr, &m_ComputeSetLayout) != VK_SUCCESS) {
             VE_CORE_FATAL("Failed to create compute descriptor set layout!");
        }

        // Allocate Descriptor Sets
        m_ComputeDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, m_ComputeSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_ComputeDescriptorPool;
        allocInfo.descriptorSetCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, m_ComputeDescriptorSets.data()) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to allocate compute descriptor sets!");
        }

        // Update Descriptor Sets (Initial binding)
        for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            // Get buffer/image infos (ensure buffers/texture exist)
            auto uboInfo = m_ComputeUboBuffers[i]->DescriptorInfo();
            
            // Output texture needs GENERAL layout for storage image access
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageView = m_OutputTexture->GetImageView();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // Use GENERAL layout
            // Sampler not needed for storage image

            auto nodeBufferInfo = m_SvoNodeBuffer ? m_SvoNodeBuffer->DescriptorInfo() : VkDescriptorBufferInfo{};
            auto voxelPalBufferInfo = m_VoxelPaletteBuffer ? m_VoxelPaletteBuffer->DescriptorInfo() : VkDescriptorBufferInfo{};
            auto materialPalBufferInfo = m_MaterialPaletteBuffer ? m_MaterialPaletteBuffer->DescriptorInfo() : VkDescriptorBufferInfo{};

            std::vector<VkWriteDescriptorSet> descriptorWrites = {
                 // Binding 0: Compute UBO
                 {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, m_ComputeDescriptorSets[i], 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &uboInfo, nullptr},
                 // Binding 1: Output Texture (Storage Image)
                 {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, m_ComputeDescriptorSets[i], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &imageInfo, nullptr, nullptr},
                 // Binding 2: Node Buffer
                 {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, m_ComputeDescriptorSets[i], 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &nodeBufferInfo, nullptr},
                 // Binding 3: Voxel Palette
                 {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, m_ComputeDescriptorSets[i], 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &voxelPalBufferInfo, nullptr},
                 // Binding 4: Material Palette
                 {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, m_ComputeDescriptorSets[i], 4, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &materialPalBufferInfo, nullptr}
            };
            
            vkUpdateDescriptorSets(m_Device.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }
    
    void VoxelRaytraceSystem::CreateComputePipeline() {
        VE_CORE_ASSERT(m_ComputeSetLayout != VK_NULL_HANDLE, "Cannot create compute pipeline without descriptor set layout!");

        // Create Pipeline Layout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_ComputeSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // No push constants for now
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_ComputePipelineLayout) != VK_SUCCESS) {
            VE_CORE_FATAL("failed to create compute pipeline layout!");
        }

        // Load Compute Shader
        // Use ShaderManager assuming it handles compute shaders
        auto computeCode = Pipeline::GetShaderManager().LoadShader("shaders/raytracer.comp", ShaderType::Compute);
        if (computeCode.empty()) {
             VE_CORE_FATAL("Failed to load compute shader: shaders/raytracer.comp");
             return;
        }
        VkShaderModule computeShaderModule;
        // Call non-static CreateShaderModule via a dummy Pipeline instance or make CreateShaderModule static
        // Assuming Pipeline instance isn't needed just for module creation based on its implementation.
        // Let's try making a static helper or calling directly on device if Pipeline's version just wraps vkCreate.
        // Replicating vkCreateShaderModule call here for simplicity:
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = computeCode.size() * sizeof(uint32_t);
        createInfo.pCode = computeCode.data();
        if (vkCreateShaderModule(m_Device.GetDevice(), &createInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to create compute shader module!");
            vkDestroyPipelineLayout(m_Device.GetDevice(), m_ComputePipelineLayout, nullptr); // Clean up layout
            m_ComputePipelineLayout = VK_NULL_HANDLE;
            return;
        }
        // Pipeline::CreateShaderModuleHelper(m_Device, computeCode, &computeShaderModule); // Removed previous attempt

        // Create Compute Pipeline
        VkComputePipelineCreateInfo computePipelineInfo{};
        computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineInfo.layout = m_ComputePipelineLayout;
        computePipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computePipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computePipelineInfo.stage.module = computeShaderModule;
        computePipelineInfo.stage.pName = "main";
        computePipelineInfo.stage.flags = 0;
        computePipelineInfo.stage.pNext = nullptr;
        computePipelineInfo.stage.pSpecializationInfo = nullptr;
        computePipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        computePipelineInfo.basePipelineIndex = -1;

        if (vkCreateComputePipelines(m_Device.GetDevice(), VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &m_ComputePipeline) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to create compute pipeline!");
        }

        // Cleanup shader module
        vkDestroyShaderModule(m_Device.GetDevice(), computeShaderModule, nullptr);
        VE_CORE_INFO("Compute pipeline created.");
    }

    void VoxelRaytraceSystem::CreateComputeResources() {
        CreateComputeUboBuffers();
        CreateComputeDescriptorSets(); // Needs UBOs and SVO buffers ready
        CreateComputePipeline(); // Needs descriptor set layout ready
    }

    // --- Public Methods ---

    void VoxelRaytraceSystem::OnResize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return; // Don't resize to zero
        if (width != m_Width || height != m_Height) {
            m_NeedsResize = true;
            m_Width = width;
            m_Height = height;
            // Note: Actual recreation of texture/buffers happens lazily in DispatchCompute now
            // We also need to update UBO resolution info
        }
    }

    // Renamed from TraceRays - this now Records compute commands
    void VoxelRaytraceSystem::DispatchCompute(VkCommandBuffer commandBuffer, uint32_t frameIndex) {
        if (m_NeedsResize) {
            // Wait for device idle before destroying old resources and creating new ones
            vkDeviceWaitIdle(m_Device.GetDevice());
            
            // Recreate Output Texture (unique_ptr handles deletion of old one)
            CreateOutputTexture(m_Width, m_Height);
            // Update display descriptor set to point to the new texture view/sampler
            // (This assumes the display pipeline uses the descriptor set created by CreateDescriptorSet)
            VkDescriptorImageInfo displayImageInfo = m_OutputTexture->GetDescriptorInfo(); 
            VkWriteDescriptorSet displayWrite = {};
            displayWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            displayWrite.dstSet = m_DescriptorSet; // Display descriptor set
            displayWrite.dstBinding = 0;
            displayWrite.dstArrayElement = 0;
            displayWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            displayWrite.descriptorCount = 1;
            displayWrite.pImageInfo = &displayImageInfo;
            vkUpdateDescriptorSets(m_Device.GetDevice(), 1, &displayWrite, 0, nullptr);

            // Recreate/Update compute descriptor sets for the new storage image view
            // (Assuming SVO buffers don't need resize here, only image changed)
            for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
                 VkDescriptorImageInfo computeImageInfo = {};
                 computeImageInfo.imageView = m_OutputTexture->GetImageView();
                 computeImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                 VkWriteDescriptorSet computeImageWrite = {};
                 computeImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                 computeImageWrite.dstSet = m_ComputeDescriptorSets[i]; // Compute descriptor set
                 computeImageWrite.dstBinding = 1; // Binding 1 for Storage Image
                 computeImageWrite.dstArrayElement = 0;
                 computeImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                 computeImageWrite.descriptorCount = 1;
                 computeImageWrite.pImageInfo = &computeImageInfo;
                 vkUpdateDescriptorSets(m_Device.GetDevice(), 1, &computeImageWrite, 0, nullptr);
            }
            
            m_NeedsResize = false;
            VE_CORE_INFO("VoxelRaytraceSystem resized resources for {}x{}", m_Width, m_Height);
        }
        
        // --- Update SVO Buffers (if world changed - needs a dirty flag mechanism) ---
        // For now, assume static world after initial upload
        // CreateOrResizeSvoBuffers(); // Check if needed
        // UpdateSvoBuffers(); // Upload if needed
        // Remember to update compute descriptor sets if buffers are recreated!

        // Update Uniform Buffer for current frame
        UpdateComputeUboBuffer(frameIndex);

        // --- Record Compute Dispatch Commands ---
        
        // Barrier: Transition output texture to GENERAL layout for compute shader write
        m_OutputTexture->TransitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

        // Bind Compute Pipeline & Descriptors
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipelineLayout, 0, 1, &m_ComputeDescriptorSets[frameIndex], 0, nullptr);

        // Dispatch
        // Calculate group counts based on image dimensions and local workgroup size (8x8)
        uint32_t groupCountX = (m_Width + 7) / 8;
        uint32_t groupCountY = (m_Height + 7) / 8;
        vkCmdDispatch(commandBuffer, groupCountX, groupCountY, 1);

        // Barrier: Transition output texture back to SHADER_READ_ONLY for sampling in graphics pipeline
        m_OutputTexture->TransitionLayout(commandBuffer, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // Simple shading function (Placeholder) - No longer called directly
    // glm::vec4 VoxelRaytraceSystem::ShadeHit(...) { ... }

    // No longer needed as GPU writes directly
    // void VoxelRaytraceSystem::UpdateOutputTexture(VkCommandBuffer commandBuffer) { ... }
    
    const Texture& VoxelRaytraceSystem::GetOutputTexture() const {
        VE_CORE_ASSERT(m_OutputTexture != nullptr, "Output texture is null!");
        return *m_OutputTexture;
    }
    
    VkDescriptorSet VoxelRaytraceSystem::GetOutputTextureDescriptorSet() const {
        return m_DescriptorSet;
    }
    
    VkDescriptorSetLayout VoxelRaytraceSystem::GetOutputTextureDescriptorSetLayout() const {
        return m_DescriptorSetLayout;
    }

} // namespace VoxelicousEngine 