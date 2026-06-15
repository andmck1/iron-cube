#pragma once

class ILayer
{
    public:
    virtual ~ILayer() = default;

    virtual bool initialise() = 0;
    virtual bool frame() = 0;
    virtual bool update() = 0;
    virtual bool postRender() = 0;
    virtual bool end() = 0;
};
