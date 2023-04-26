#pragma once

#ifdef VE_PLATFORM_WINDOWS

extern VoxelicousEngine::App* VoxelicousEngine::CreateApp();

int main(int argc, char** argv)
{
	auto app = VoxelicousEngine::CreateApp();
	app->Run();
	delete app;
}

#endif