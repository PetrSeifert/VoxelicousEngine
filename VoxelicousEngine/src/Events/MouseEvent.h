#pragma once

#include "Event.h"

namespace VoxelicousEngine
{
    class MouseMovedEvent final : public Event
    {
    public:
        explicit MouseMovedEvent(const float x, const float y)
            : m_MouseX(x), m_MouseY(y)
        {
        }

        ~MouseMovedEvent() override = default;

        float GetX() const { return m_MouseX; }
        float GetY() const { return m_MouseY; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float m_MouseX;
        float m_MouseY;
    };

    class MouseScrolledEvent final : public Event
    {
    public:
        explicit MouseScrolledEvent(const float xOffset, const float yOffset)
            : m_XOffset(xOffset), m_YOffset(yOffset)
        {
        }

        float GetXOffset() const { return m_XOffset; }
        float GetYOffset() const { return m_YOffset; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float m_XOffset;
        float m_YOffset;
    };

    class MouseButtonEvent : public Event
    {
    public:
        int GetMouseButton() const { return m_Button; }

        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    protected:
        explicit MouseButtonEvent(const int button)
            : m_Button(button)
        {
        }

    private:
        int m_Button;
    };

    class MouseButtonPressedEvent final : public MouseButtonEvent
    {
    public:
        explicit MouseButtonPressedEvent(const int button)
            : MouseButtonEvent(button)
        {
        }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << GetMouseButton();
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent final : public MouseButtonEvent
    {
    public:
        explicit MouseButtonReleasedEvent(const int button)
            : MouseButtonEvent(button)
        {
        }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: " << GetMouseButton();
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };
}
