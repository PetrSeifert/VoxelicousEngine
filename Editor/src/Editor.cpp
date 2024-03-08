#include <VoxelicousEngine.h>

const VoxelicousEngine::Model::Builder sprite = { {
	{ {-1, 0, -1}, {.0f, 1.0f, .0f} },
	{ {-1, 0, -1}, {0.0f, 1.0f, 0.0f} },
	{ {1, 0, -1}, {0.0f, 1.0f, 0.0f} },
	{ {1, 0, -1}, {0.0f, 1.0f, 0.0f} },
	{ {-1, 0, 1}, {0.0f, 1.0f, 0.0f} },
	{ {-1, 0, 1}, {0.0f, 1.0f, 0.0f} },
	{ {1, 0, 1}, {0.0f, 1.0f, 0.0f} },
	{ {1, 0, 1}, {0.0f, 1.0f, 0.0f} }},
	{0, 1, 2, 0, 2, 3,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7,
	5, 4, 7, 5, 7, 6}
};

struct GlobalUbo 
{
	glm::mat4 Projection{ 1.f };
	glm::mat4 View{ 1.f };
	glm::vec4 AmbientLightColor{ 1.f, 1.f, 1.f, .02f };
	glm::vec3 LightPosition{ -5.f, -6.f, 0.f };
	alignas(16) glm::vec4 LightColor{ 1.f };
};

class ExampleLayer final : public VoxelicousEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	/*void OnAttach() override
	{
		m_UboBuffers = std::vector<std::unique_ptr<Buffer>>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (auto& m_UboBuffer : m_UboBuffers)
		{
			m_UboBuffer = std::make_unique<Buffer>(
				m_Device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			m_UboBuffer->Map();
		}

		m_GlobalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < m_GlobalDescriptorSets.size(); i++) {
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
	}*/

	void OnUpdate(VkCommandBuffer commandBuffer) override
	{
		VE_INFO("ExampleLayer::Update");
	}

	void OnEvent(VoxelicousEngine::Event& event) override
	{
		VE_TRACE("{0}", event);
	}
};

class FirstApp : public VoxelicousEngine::App
{
public:
	FirstApp()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new VoxelicousEngine::ImGuiLayer(*m_Renderer, *m_Device, *m_GlobalPool));
		PushLayer(new VoxelicousEngine::DefaultLayer(*m_Renderer, *m_Device, *m_GlobalPool));
	}

	~FirstApp() override = default;
};

VoxelicousEngine::App* VoxelicousEngine::CreateApp()
{
	return new FirstApp();
}