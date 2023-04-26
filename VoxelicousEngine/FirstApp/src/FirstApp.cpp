#include <VoxelicousEngine/VoxelicousEngine.h>

class FirstApp : public VoxelicousEngine::App
{
public:
	FirstApp()
	{

	}

	~FirstApp()
	{

	}

};

VoxelicousEngine::App* VoxelicousEngine::CreateApp()
{
	return new FirstApp();
}