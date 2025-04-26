set(PROJECT_NAME VoxelicousEngine)

# Get all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${IMGUI_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/submodules/spdlog/include
        ${VULKAN_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/submodules/glfw/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        imgui
        glfw
        ${VULKAN_LIB}
)

# Define preprocessor macros
target_compile_definitions(${PROJECT_NAME} PRIVATE VE_BUILD_DLL)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VE_PLATFORM_WINDOWS)
    if(MINGW)
        target_link_libraries(${PROJECT_NAME} 
            PUBLIC 
                ws2_32
                winmm
                version
                bcrypt
        )
    else()
        target_link_libraries(${PROJECT_NAME} 
            PUBLIC 
                Ws2_32.lib
                Winmm.lib
                Version.lib
                Bcrypt.lib
        )
    endif()
endif()

# Configuration-specific settings
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:VE_DEBUG NDEBUG>
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