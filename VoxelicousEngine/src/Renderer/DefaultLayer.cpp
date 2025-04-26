#include "vepch.h"
#include "DefaultLayer.h"
#include "Core/Model.h"
#include "Core/GameObject.h"

namespace VoxelicousEngine
{
    const Model::Builder VOXEL = {
        {
            {{-1, -1, -1}, {.0f, 1.0f, .0f}},
            {{-1, 1, -1}, {0.0f, 1.0f, 0.0f}},
            {{1, 1, -1}, {0.0f, 1.0f, 0.0f}},
            {{1, -1, -1}, {0.0f, 1.0f, 0.0f}},
            {{-1, -1, 1}, {0.0f, 1.0f, 0.0f}},
            {{-1, 1, 1}, {0.0f, 1.0f, 0.0f}},
            {{1, 1, 1}, {0.0f, 1.0f, 0.0f}},
            {{1, -1, 1}, {0.0f, 1.0f, 0.0f}}
        },
        {
            0, 1, 2, 0, 2, 3,
            4, 5, 1, 4, 1, 0,
            3, 2, 6, 3, 6, 7,
            1, 5, 6, 1, 6, 2,
            4, 0, 3, 4, 3, 7,
            5, 4, 7, 5, 7, 6
        }
    };

    struct GlobalUbo
    {
        glm::mat4 Projection{1.f};
        glm::mat4 View{1.f};
        glm::vec4 AmbientLightColor{1.f, 1.f, 1.f, .02f};
        glm::vec3 LightPosition{-5.f, -6.f, 0.f};
        alignas(16) glm::vec4 LightColor{1.f};
    };

    DefaultLayer::DefaultLayer(Renderer& renderer, Device& device, DescriptorPool& globalPool) : Layer("DefaultLayer"),
        m_Renderer(renderer), m_Device(device), m_GlobalPool(globalPool)
    {
    }

    DefaultLayer::~DefaultLayer() = default;

    void DefaultLayer::OnAttach()
    {
        m_UboBuffers = std::vector<std::unique_ptr<Buffer>>(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto& uboBuffer : m_UboBuffers)
        {
            uboBuffer = std::make_unique<Buffer>(
                m_Device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffer->Map();
        }

        m_GlobalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < m_GlobalDescriptorSets.size(); i++)
        {
            auto bufferInfo = m_UboBuffers[i]->DescriptorInfo();
            DescriptorWriter(*m_GlobalSetLayout, m_GlobalPool)
                .WriteBuffer(0, &bufferInfo)
                .Build(m_GlobalDescriptorSets[i]);
        }

        const auto myModel = std::make_shared<Model>(m_Device, VOXEL);
        const auto myModel1 = std::make_shared<Model>(m_Device, VOXEL);
        const auto myModel2 = std::make_shared<Model>(m_Device, VOXEL);

        auto gameObj = GameObject::CreateGameObject();
        gameObj.Model = myModel;
        gameObj.Transform.Translation = {0, 0, 0};
        m_GameObjects.emplace(gameObj.GetId(), std::move(gameObj));

        auto gameObj1 = GameObject::CreateGameObject();
        gameObj1.Model = myModel1;
        gameObj1.Transform.Translation = {1, 0, 0};
        m_GameObjects.emplace(gameObj1.GetId(), std::move(gameObj1));

        auto gameObj2 = GameObject::CreateGameObject();
        gameObj2.Model = myModel2;
        gameObj2.Transform.Translation = {2, 0, 0};
        m_GameObjects.emplace(gameObj2.GetId(), std::move(gameObj2));
    }

    void DefaultLayer::OnDetach()
    {
    }

    void DefaultLayer::OnUpdate(const VkCommandBuffer commandBuffer)
    {
        const auto newTime = std::chrono::steady_clock::now();
        const float frameTime = std::chrono::duration<float>(newTime - m_CurrentTime).count();
        m_CurrentTime = newTime;
        m_CameraController.MoveInPlaneXZ(m_Window.GetGLFW_Window(), frameTime, m_ViewerObject);
        m_Camera.SetViewYXZ(m_ViewerObject.Transform.Translation, m_ViewerObject.Transform.Rotation);

        const float aspect = m_Renderer.GetAspectRatio();
        m_Camera.SetPerspectiveProjection(glm::radians(60.f), aspect, .1f, 100.f);

        const int frameIndex = m_Renderer.GetFrameIndex();
        const FrameInfo frameInfo
        {
            frameIndex,
            frameTime,
            commandBuffer,
            m_Camera,
            m_GlobalDescriptorSets[frameIndex],
            m_GameObjects
        };

        //update
        GlobalUbo ubo{};
        ubo.Projection = m_Camera.GetProjection();
        ubo.View = m_Camera.GetView();
        m_UboBuffers[frameIndex]->WriteToBuffer(&ubo);
        m_UboBuffers[frameIndex]->Flush();

        //render
        m_SimpleRendererSystem.RenderGameObjects(frameInfo);
    }

    void DefaultLayer::OnEvent(Event& event)
    {
    }
}
