set(PROJECT_NAME Editor)

# Get all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SHADERS "shaders/*.vert" "shaders/*.frag")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

# Set include directories
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/VoxelicousEngine/submodules/spdlog/include
        ${CMAKE_SOURCE_DIR}/VoxelicousEngine/src
        ${GLM_INCLUDE_DIR}
        ${VULKAN_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/VoxelicousEngine/submodules/glfw/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        VoxelicousEngine
)

# Define preprocessor macros
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VE_PLATFORM_WINDOWS)
endif()

# Configuration-specific settings
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:VE_DEBUG>
    $<$<CONFIG:Release>:VE_RELEASE>
    $<$<CONFIG:Dist>:VE_DIST>
)

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}"
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}"
)

# Set Debug/Release runtime library
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:Dist>:/MD>
    )
elseif(MINGW)
    # MinGW specific flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Dist>:-O2>
    )
endif()

# Copy shaders to output directory (directly next to the executable)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}/shaders"
) 