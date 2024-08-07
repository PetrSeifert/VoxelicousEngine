#include "vepch.h"
#include "Layer.h"

namespace VoxelicousEngine
{
    Layer::Layer(std::string name)
        : m_DebugName(std::move(name))
    {
    }
}