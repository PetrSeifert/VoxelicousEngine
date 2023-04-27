#include <VoxelicousEngine/VoxelicousEngine.h>

class ExampleLayer : public VoxelicousEngine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
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
		PushLayer(new ExampleLayer());
	}

	~FirstApp()
	{

	}

};

VoxelicousEngine::App* VoxelicousEngine::CreateApp()
{
	return new FirstApp();
}