project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/VoxelicousEngine/submodules/spdlog/include",
		"%{wks.location}/VoxelicousEngine/src",
		"%{wks.location}/VoxelicousEngine/submodules",
		"%{IncludeDir.glm}",
		"%{IncludeDir.VulkanSDK}",
	}

	links
	{
		"VoxelicousEngine"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "VE_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		defines "VE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "VE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "VE_DIST"
		runtime "Release"
		optimize "on"