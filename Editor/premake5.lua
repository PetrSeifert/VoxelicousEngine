project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

	files
    {
        "src/**.h",
        "src/**.cpp",
		"shaders/**.vert",
		"shaders/**.frag"
    }

    includedirs
    {
        "%{wks.location}/VoxelicousEngine/submodules/spdlog/include",
        "%{wks.location}/VoxelicousEngine/src",
        "%{IncludeDir.glm}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.GLFW}",
    }

    links
    {
        "VoxelicousEngine",
    }

    -- Windows-specific settings
    filter "system:windows"
        systemversion "latest"
        defines { "VE_PLATFORM_WINDOWS" }

    -- Debug configuration settings
    filter "configurations:Debug"
        defines "VE_DEBUG"
        runtime "Debug"
        symbols "on"

    -- Release configuration settings
    filter "configurations:Release"
        defines "VE_RELEASE"
        runtime "Release"
        optimize "on"

    -- Distribution configuration settings
    filter "configurations:Dist"
        defines "VE_DIST"
        runtime "Release"
        optimize "on"
