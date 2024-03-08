#pragma once
#ifdef VE_PLATFORM_WINDOWS

extern auto VoxelicousEngine::CreateApp() -> VoxelicousEngine::App*;

inline auto main(int  /*argc*/, char**  /*argv*/) -> int
{
    VoxelicousEngine::Log::Init();
    VE_CORE_WARN("Init Log!");

    auto *const app = VoxelicousEngine::CreateApp();
    app->Run();
    delete app;
}

#endif
