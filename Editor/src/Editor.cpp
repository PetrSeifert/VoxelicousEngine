#include <VoxelicousEngine.h>

const VoxelicousEngine::Model::Builder CUBE = { {
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

class FirstApp : public VoxelicousEngine::App
{
public:
	FirstApp()
	{
		PushLayer(new VoxelicousEngine::ImGuiLayer(*m_Renderer, *m_Device, *m_GlobalPool));
		PushLayer(new VoxelicousEngine::DefaultLayer(*m_Renderer, *m_Device, *m_GlobalPool));
	}

	~FirstApp() override = default;
};

VoxelicousEngine::App* VoxelicousEngine::CreateApp()
{
	return new FirstApp();
}