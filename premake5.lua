include "./submodules/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "VoxelicousEngine"
	architecture "x86_64"
	startproject "Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "submodules/premake"
	include "VoxelicousEngine/submodules/glfw"
	include "VoxelicousEngine/submodules/imgui"
group ""

group "Core"
	include "VoxelicousEngine"
group ""

group "Misc"
	include "Editor"
group ""