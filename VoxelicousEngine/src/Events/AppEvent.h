#pragma once

#include "Event.h"

namespace VoxelicousEngine
{
    class WindowResizeEvent final : public Event
    {
    public:
        WindowResizeEvent(const unsigned int width, const unsigned int height)
            : m_Width(width), m_Height(height)
        {
        }

        ~WindowResizeEvent() override = default ;

        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApp)

    private:
        unsigned int m_Width;
        unsigned int m_Height;
    };

    class WindowCloseEvent final : public Event
    {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApp)
    };

    class AppTickEvent final : public Event
    {
    public:
        AppTickEvent() = default;

        EVENT_CLASS_TYPE(AppTick)
        EVENT_CLASS_CATEGORY(EventCategoryApp)
    };

    class AppUpdateEvent final : public Event
    {
    public:
        AppUpdateEvent() = default;

        EVENT_CLASS_TYPE(AppUpdate)
        EVENT_CLASS_CATEGORY(EventCategoryApp)
    };

    class AppRenderEvent final : public Event
    {
    public:
        AppRenderEvent() = default;

        EVENT_CLASS_TYPE(AppRender)
        EVENT_CLASS_CATEGORY(EventCategoryApp)
    };
}
