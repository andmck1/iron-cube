#pragma once

class ILayer
{
    public:
    virtual ~ILayer() = default;

    virtual bool initialise() = 0;

    virtual void render() = 0;

    virtual void shutdown() = 0;
};
