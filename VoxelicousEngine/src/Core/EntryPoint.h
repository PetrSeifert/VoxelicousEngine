#ifdef VE_PLATFORM_WINDOWS

extern VoxelicousEngine::App* VoxelicousEngine::CreateApp();

int main(int argc, char** argv)
{
	VoxelicousEngine::Log::Init();
	VE_CORE_WARN("Init Log!");

	auto app = VoxelicousEngine::CreateApp();
	app->Run();
	delete app;
}

#endif