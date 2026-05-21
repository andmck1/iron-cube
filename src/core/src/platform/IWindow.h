#pragma once

#include <cstdint>

struct Vec2u
{
    uint32_t x;
    uint32_t y;
};

class IWindow
{
    public:
    virtual ~IWindow() = default;

    virtual bool isOpen() const = 0;

    virtual bool isIconified() const = 0;

    virtual void requestClose() = 0;

    virtual Vec2u getSize() const = 0;
};
