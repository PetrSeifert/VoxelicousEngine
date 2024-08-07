#pragma once

namespace VoxelicousEngine
{
    class TimeStep
    {
    public:
        explicit TimeStep(const float time = 0.0f)
            : m_Time(time)
        {
        }

        float GetSeconds() const { return m_Time; }
        
        float GetMilliseconds() const { return m_Time * 1000.0f; }

    private:
        float m_Time;
    };
}
