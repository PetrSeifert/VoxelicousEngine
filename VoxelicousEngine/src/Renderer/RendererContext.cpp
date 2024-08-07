#include "vepch.h"
#include "RendererContext.h"

#include "Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace VoxelicousEngine {

    Ref<RendererContext> RendererContext::Create()
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::None:    return nullptr;
        case RendererAPIType::Vulkan:  return Ref<VulkanContext>::Create();
        }
        VE_CORE_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

}