project "VoxelicousEngine"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "src",

      "submodules/imgui",
      "submodules/glfw/include",
      "submodules/spdlog/include",

      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.glm}",
   }

   links
   {
       "ImGui",
       "GLFW",

       "%{Library.Vulkan}",
   }

   targetdir ("bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")
   defines { "VE_BUILD_DLL"}

   filter "system:windows"
      systemversion "latest"
      defines { "VE_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
      defines { "VE_DEBUG", "NDEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "VE_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      defines { "VE_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"