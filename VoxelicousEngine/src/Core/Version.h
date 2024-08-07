#pragma once

#define VE_VERSION "2024.1.1"

//
// Build Configuration
//
#if defined(VE_DEBUG)
    #define VE_BUILD_CONFIG_NAME "Debug"
#elif defined(VE_RELEASE)
    #define VE_BUILD_CONFIG_NAME "Release"
#elif defined(VE_DIST)
    #define VE_BUILD_CONFIG_NAME "Dist"
#else
    #error Undefined configuration?
#endif

//
// Build Platform
//
#if defined(VE_PLATFORM_WINDOWS)
    #define VE_BUILD_PLATFORM_NAME "Windows x64"
#elif defined(VE_PLATFORM_LINUX)
    #define VE_BUILD_PLATFORM_NAME "Linux"
#else
    #define VE_BUILD_PLATFORM_NAME "Unknown"
#endif

#define VE_VERSION_LONG "Voxelicous Engine " VE_VERSION " (" VE_BUILD_PLATFORM_NAME " " VE_BUILD_CONFIG_NAME ")"