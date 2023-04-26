#pragma once

#ifdef VE_PLATFORM_WINDOWS
	#ifdef VE_BUILD_DLL
		#define VOXELICOUS_ENGINE_API __declspec(dllexport)
	#else
		#define VOXELICOUS_ENGINE_API __declspec(dllimport)
	#endif
#else
#error Voxelicous Engine only supports Windows!
#endif