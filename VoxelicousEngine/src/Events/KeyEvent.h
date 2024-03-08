#pragma once

#include "Event.h"


namespace VoxelicousEngine
{
    class KeyEvent : public Event
    {
    public:
        int GetKeyCode() const { return m_KeyCode; }

        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

    protected:
        explicit KeyEvent(const int keycode)
            : m_KeyCode(keycode)
        {
        }

    private:
        int m_KeyCode;
    };

    class KeyPressedEvent final : public KeyEvent
    {
    public:
        KeyPressedEvent(const int keycode, const int repeatCount)
            : KeyEvent(keycode), m_RepeatCount(repeatCount)
        {
        }

        int GetRepeatCount() const { return m_RepeatCount; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << GetKeyCode() << " (" << m_RepeatCount << " repeats)";
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyPressed)

    private:
        int m_RepeatCount;
    };

    class KeyReleasedEvent final : public KeyEvent
    {
    public:
        explicit KeyReleasedEvent(const int keycode)
            : KeyEvent(keycode)
        {
        }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << GetKeyCode();
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyReleased)
    };
}
