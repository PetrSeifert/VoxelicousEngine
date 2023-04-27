#pragma once

#include "Core.h"
#include "Log.h"

namespace VoxelicousEngine {

	class VOXELICOUS_ENGINE_API App
	{
	public:
		App();
		virtual ~App();

		void Run();

	};

	// To be defined in CLIENT
	App* CreateApp();
}
