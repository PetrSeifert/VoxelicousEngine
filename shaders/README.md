# Shader System

This folder contains all shaders used by the Voxelicious Engine. The engine supports automatic GLSL shader compilation using the SpirvHelper and ShaderManager classes.

## Shader Files

The following types of shader files are supported:
- `.vert` - Vertex shaders
- `.frag` - Fragment shaders
- `.comp` - Compute shaders (not currently used)

## Automatic Compilation

Shaders are automatically compiled to SPIR-V when loaded by the engine. The process works as follows:

1. When a shader is requested through `Pipeline` and `ShaderManager`, the system first checks if the source GLSL file has changed since the last compilation.
2. If the shader has changed or hasn't been compiled yet, it is compiled to SPIR-V using the shaderc library.
3. The compiled SPIR-V code is cached in memory and also saved to disk with a `.spv` extension.
4. The compiled shader is then used for rendering.

## Hot Reloading

The engine supports hot reloading of shaders:

1. Edit a shader file (e.g., `simple.vert` or `simple.frag`)
2. Save the file
3. The engine will detect the change automatically and recompile the shader
4. The rendering will update without needing to restart the application

This is handled by the `ShaderManager::CheckForChanges()` method, which is called at the beginning of each frame render.

## Adding New Shaders

To add a new shader:

1. Create a new file with the appropriate extension (`.vert`, `.frag`, etc.)
2. Write your GLSL shader code
3. Load the shader through the Pipeline class:

```cpp
// Create a pipeline with your new shaders
m_Pipeline = std::make_unique<Pipeline>(
    m_Device,
    "shaders/your_new_shader.vert",  // Path relative to shader directory
    "shaders/your_new_shader.frag",
    pipelineConfig
);
```

The engine will automatically compile and use your shader. 