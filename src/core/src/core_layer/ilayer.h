#pragma once

class ILayer
{
    public:
    virtual ~ILayer() = default;

    virtual bool initialise() = 0;
    virtual bool frame() = 0;
};
