project "VoxelicousEngine"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   staticruntime "off"

   files 
   { 
        "src/**.h", 
        "src/**.cpp",
   }

   includedirs
   {
      "src",

      "submodules/imgui",
      "submodules/spdlog/include",
       
      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.glm}",
      "%{IncludeDir.GLFW}",
   }

   links
   {
      "ImGui",
      "GLFW",
      "%{Library.Vulkan}",
   }

   targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
   objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
   debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
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