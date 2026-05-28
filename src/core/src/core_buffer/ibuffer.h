#pragma once

class IBuffer
{
    public:
    virtual ~IBuffer() = default;

    virtual void init() = 0;
    virtual void set() = 0;
};
