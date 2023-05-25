#include <VoxelicousEngine.h>

class ExampleLayer : public VoxelicousEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

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

	~FirstApp()
	{

	}

};

VoxelicousEngine::App* VoxelicousEngine::CreateApp()
{
	return new FirstApp();
}