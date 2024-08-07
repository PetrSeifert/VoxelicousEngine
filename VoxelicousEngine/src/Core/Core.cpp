#include "vepch.h"
#include "Core.h"

#include "Log.h"
#include "Memory.h"
#include "Version.h"

namespace VoxelicousEngine
{
    void InitializeCore()
    {
        Allocator::Init();
        Log::Init();

        VE_CORE_TRACE_TAG("Core", "Voxelicous Engine {}", VE_VERSION);
        VE_CORE_TRACE_TAG("Core", "Initializing...");
    }

    void ShutdownCore()
    {
        VE_CORE_TRACE_TAG("Core", "Shutting down...");
		
        Log::Shutdown();
    }

}
