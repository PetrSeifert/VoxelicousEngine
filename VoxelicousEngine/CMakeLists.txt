cmake_minimum_required (VERSION 3.8)

project ("VoxelicousEngine" LANGUAGES CXX)


set(VOXELICOUS_ENGINE_HEADERS
	"${VOXELICOUS_ENGINE_SRC_DIR}/VoxelicousEngine/VoxelicousEngine.h"
	"${VOXELICOUS_ENGINE_SRC_DIR}/VoxelicousEngine/Core/App.h"
	"${VOXELICOUS_ENGINE_SRC_DIR}/VoxelicousEngine/Core/Core.h"
	"${VOXELICOUS_ENGINE_SRC_DIR}/VoxelicousEngine/Core/EntryPoint.h"
)

set(VOXELICOUS_ENGINE_SOURCES
	"${VOXELICOUS_ENGINE_SRC_DIR}/VoxelicousEngine/Core/App.cpp"
)

add_library(VoxelicousEngine SHARED ${VOXELICOUS_ENGINE_SOURCES} ${VOXELICOUS_ENGINE_HEADERS})

target_include_directories(VoxelicousEngine PUBLIC "${VOXELICOUS_ENGINE_SRC_DIR}")
target_compile_definitions(VoxelicousEngine PRIVATE VE_PLATFORM_WINDOWS VE_BUILD_DLL)

set_target_properties(VoxelicousEngine PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${DEFAULT_DIR}/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>/VoxelicousEngine/Runtime"
	LIBRARY_OUTPUT_DIRECTORY "${DEFAULT_DIR}/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>/VoxelicousEngine"
    ARCHIVE_OUTPUT_DIRECTORY "${DEFAULT_DIR}/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>/VoxelicousEngine/Archive"
	LINKER_LANGUAGE CXX
	CXX_STANDARD 20
	OUTPUT_NAME "VoxelicousEngine"
)
target_compile_options(VoxelicousEngine PRIVATE /MDd)