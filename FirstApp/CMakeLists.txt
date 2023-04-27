cmake_minimum_required (VERSION 3.8)

project ("FirstApp" LANGUAGES CXX)

add_executable(FirstApp ${FIRSTAPP_SRC_DIR}/FirstApp.cpp)

target_include_directories(FirstApp PRIVATE "${VOXELICOUS_ENGINE_SRC_DIR}" "${DEFAULT_DIR}/VoxelicousEngine/submodules/spdlog/include")
target_link_libraries(FirstApp PRIVATE "${DEFAULT_DIR}/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>/VoxelicousEngine/Archive/VoxelicousEngine.lib")
target_compile_definitions(FirstApp PRIVATE VE_PLATFORM_WINDOWS)
set_target_properties(FirstApp PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${DEFAULT_DIR}/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>/VoxelicousEngine/Runtime"
	LIBRARY_OUTPUT_DIRECTORY "${DEFAULT_DIR}/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>/VoxelicousEngine"
    ARCHIVE_OUTPUT_DIRECTORY "${DEFAULT_DIR}/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/$<CONFIG>/VoxelicousEngine/Archive"
)
target_compile_options(FirstApp PRIVATE /MDd)