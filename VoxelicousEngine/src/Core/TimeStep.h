#pragma once

namespace VoxelicousEngine
{
    class TimeStep
    {
    public:
        TimeStep(): m_Time(0){}
        TimeStep(float time = 0.0f);

        float GetSeconds() const { return m_Time; }
        float GetMilliseconds() const { return m_Time * 1000.0f; }

        operator float() const { return m_Time; }
    private:
        float m_Time;
    };
}