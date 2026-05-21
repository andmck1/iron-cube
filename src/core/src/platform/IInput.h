#pragma once

#include <cstdint>

enum class KeyCode : int32_t
{
    Escape = 256,
};

class IInput
{
    public:
    virtual ~IInput() = default;

    virtual bool isKeyPressed(KeyCode key) const = 0;
};
