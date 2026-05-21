#pragma once

#include <unordered_map>
#include <typeindex>

class ServiceLocator
{
    public:
    template <typename T> void registerService(T *service)
    {
        services_[std::type_index(typeid(T))] = service;
    }

    template <typename T> T *get()
    {
        auto services_iterator = services_.find(std::type_index(typeid(T)));
        if (services_iterator != services_.end())
            return static_cast<T *>(services_iterator->second);
        return nullptr;
    }

    static ServiceLocator &instance();

    private:
    std::unordered_map<std::type_index, void *> services_;
};
