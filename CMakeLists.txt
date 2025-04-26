cmake_minimum_required(VERSION 3.16)
project(VoxelicousEngine)

# Define build configurations upfront
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)

# Set flags for Dist configuration (required)
set(CMAKE_CXX_FLAGS_DIST "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DIST "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DIST "${CMAKE_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_DIST "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LINKER_FLAGS_DIST "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)

# Set default build type if not multi-configuration generator
if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release Dist" FORCE)
  endif()
  # Make sure CMAKE_BUILD_TYPE is one of our allowed types
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release Dist)
endif()

# MinGW specific settings
if(MINGW)
    # Disable specific warnings that might occur with MinGW
    add_compile_options(-Wno-deprecated-declarations -Wno-unknown-pragmas -Wno-declaration-after-statement -Wno-incompatible-pointer-types)
    
    # Define UNICODE for Win32 API
    add_definitions(-DUNICODE -D_UNICODE)
    
    # For GLFW compatibility with MinGW
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories with build type
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

# Global compile options
if(MSVC)
    add_compile_options(/MP)
endif()

# Find Vulkan SDK
set(VULKAN_SDK $ENV{VULKAN_SDK})
if(NOT VULKAN_SDK)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK and set VULKAN_SDK environment variable.")
endif()

# Set include directories
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/VoxelicousEngine/submodules)
set(GLM_INCLUDE_DIR ${INCLUDE_DIR}/glm)
set(IMGUI_INCLUDE_DIR ${INCLUDE_DIR}/imgui)
set(VULKAN_INCLUDE_DIR ${VULKAN_SDK}/Include)

# Set library directories
if(MINGW)
    set(VULKAN_LIBRARY_DIR ${VULKAN_SDK}/Lib)
    set(VULKAN_LIB "vulkan-1")
    
    # Add Vulkan library directory to the linker path
    link_directories(${VULKAN_LIBRARY_DIR})
else()
    set(VULKAN_LIBRARY_DIR ${VULKAN_SDK}/Lib)
    set(VULKAN_LIB "${VULKAN_LIBRARY_DIR}/vulkan-1.lib")
endif()

# GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Dependencies
add_subdirectory(VoxelicousEngine/submodules/glfw)
add_subdirectory(VoxelicousEngine/submodules/imgui)

# Core
add_subdirectory(VoxelicousEngine)

# Applications
add_subdirectory(Editor) 