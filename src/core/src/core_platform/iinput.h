#pragma once

#include <cstdint>

enum class KeyCode : int32_t
{
    Escape = 256,
};

enum class MouseButton : int32_t
{
    Left = 0,
    Middle = 2,
    Right = 1,
};

struct MouseDelta
{
    float x = 0.0f;
    float y = 0.0f;
};

class IInput
{
    public:
    virtual ~IInput() = default;

    virtual bool isKeyPressed(KeyCode key) const = 0;
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual MouseDelta getMouseDelta() const = 0;
    virtual void resetMouseDelta() = 0;
};
