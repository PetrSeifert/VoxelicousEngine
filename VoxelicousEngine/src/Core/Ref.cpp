#include "vepch.h"

#include <unordered_set>

namespace VoxelicousEngine
{
    static std::unordered_set<void*> s_LiveReferences;
    static std::mutex s_LiveReferenceMutex;

    namespace RefUtils
    {
        void AddToLiveReferences(void* instance)
        {
            std::scoped_lock lock(s_LiveReferenceMutex);
            VE_CORE_ASSERT(instance);
            s_LiveReferences.insert(instance);
        }

        void RemoveFromLiveReferences(void* instance)
        {
            std::scoped_lock lock(s_LiveReferenceMutex);
            VE_CORE_ASSERT(instance);
            VE_CORE_ASSERT(s_LiveReferences.contains(instance));
            s_LiveReferences.erase(instance);
        }

        bool IsLive(void* instance)
        {
            VE_CORE_ASSERT(instance);
            return s_LiveReferences.contains(instance);
        }
    }
}