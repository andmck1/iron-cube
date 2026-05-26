#pragma once

#include <spdlog/spdlog.h>
#include <unordered_map>
#include <typeindex>

class ServiceLocator
{
    public:
    template <typename T> void registerService(T *service)
    {
        spdlog::debug("Registering service: {}", typeid(T).name());
        services_[std::type_index(typeid(T))] = service;
    }

    template <typename T> T *get()
    {
        spdlog::debug("Requesting service: {}", typeid(T).name());
        auto services_iterator = services_.find(std::type_index(typeid(T)));
        if (services_iterator != services_.end())
        {
            spdlog::debug("Service found: {}", typeid(T).name());
            return static_cast<T *>(services_iterator->second);
        }
        spdlog::error("Service not found: {}", typeid(T).name());
        return nullptr;
    }

    static ServiceLocator &instance();

    private:
    std::unordered_map<std::type_index, void *> services_;
};
