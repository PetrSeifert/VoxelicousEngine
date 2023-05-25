#include "vepch.h"
#include "DefaultLayer.h"
#include "Core/Model.h"
#include "Core/GameObject.h"

namespace VoxelicousEngine
{
	const Model::Builder voxel = { {
	{ {-1, -1, -1}, {.0f, 1.0f, .0f} },
	{ {-1, 1, -1}, {0.0f, 1.0f, 0.0f} },
	{ {1, 1, -1}, {0.0f, 1.0f, 0.0f} },
	{ {1, -1, -1}, {0.0f, 1.0f, 0.0f} },
	{ {-1, -1, 1}, {0.0f, 1.0f, 0.0f} },
	{ {-1, 1, 1}, {0.0f, 1.0f, 0.0f} },
	{ {1, 1, 1}, {0.0f, 1.0f, 0.0f} },
	{ {1, -1, 1}, {0.0f, 1.0f, 0.0f} }},
	{{0}, {1}, {2}, {0}, {2}, {3},
	{4}, {5}, {1}, {4}, {1}, {0},
	{3}, {2}, {6}, {3}, {6}, {7},
	{1}, {5}, {6}, {1}, {6}, {2},
	{4}, {0}, {3}, {4}, {3}, {7},
	{5}, {4}, {7}, {5}, {7}, {6}}
	};

	struct GlobalUbo 
	{
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };
		glm::vec3 lightPosition{ -5.f, -6.f, 0.f };
		alignas(16) glm::vec4 lightColor{ 1.f };
	};

	DefaultLayer::DefaultLayer(Renderer& renderer, Device& device, DescriptorPool& globalPool) : Layer("DefaultLayer"), m_Renderer(renderer), m_Device(device), m_GlobalPool(globalPool)
	{
	}

	DefaultLayer::~DefaultLayer() = default;

	void DefaultLayer::OnAttach()
	{
		m_UboBuffers = std::vector<std::unique_ptr<Buffer>>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < m_UboBuffers.size(); i++) 
		{
			m_UboBuffers[i] = std::make_unique<Buffer>(
				m_Device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			m_UboBuffers[i]->Map();
		}

		m_GlobalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < m_GlobalDescriptorSets.size(); i++) {
			auto bufferInfo = m_UboBuffers[i]->DescriptorInfo();
			DescriptorWriter(*m_GlobalSetLayout, m_GlobalPool)
				.WriteBuffer(0, &bufferInfo)
				.Build(m_GlobalDescriptorSets[i]);
		}

		auto myModel = std::make_shared<Model>(m_Device, voxel);

		auto gameObj = GameObject::CreateGameObject();
		gameObj.model = myModel;
		gameObj.transform.translation = { 0, 0, 0 };
		m_GameObjects.emplace(gameObj.getId(), std::move(gameObj));
	}

	void DefaultLayer::OnDetach()
	{
	}

	void DefaultLayer::OnUpdate(VkCommandBuffer commandBuffer)
	{
		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - m_CurrentTime).count();
		m_CurrentTime = newTime;
		cameraController.MoveInPlaneXZ(m_Window.GetGLFWwindow(), frameTime, viewerObject);
		camera.SetViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		float aspect = m_Renderer.GetAspectRatio();
		camera.SetPerspectiveProjection(glm::radians(60.f), aspect, .1f, 100.f);

		int frameIndex = m_Renderer.GetFrameIndex();
		FrameInfo frameInfo
		{
			frameIndex,
			frameTime,
			commandBuffer,
			camera,
			m_GlobalDescriptorSets[frameIndex],
			m_GameObjects
		};

		//update
		GlobalUbo ubo{};
		ubo.projection = camera.GetProjection();
		ubo.view = camera.GetView();
		m_UboBuffers[frameIndex]->WriteToBuffer(&ubo);
		m_UboBuffers[frameIndex]->Flush();

		//render
		m_SimpleRendererSystem.RenderGameObjects(frameInfo);
	}

	void DefaultLayer::OnEvent(Event& event)
	{
	}
}