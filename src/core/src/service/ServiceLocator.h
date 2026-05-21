/**
 * @file
 * @brief Service locator for dependency injection.
 *
 * Provides a global registry for platform, renderer, and other
 * services, allowing components to resolve dependencies by
 * interface type without compile-time coupling.
 */

#pragma once

#include <unordered_map>
#include <typeindex>

/**
 * @brief Global service registry.
 *
 * Stores service instances keyed by their interface type.
 * Uses a singleton pattern for simple access throughout
 * the application.
 *
 * @par Thread safety
 * Not thread-safe. Intended for single-threaded use during
 * application startup and shutdown.
 *
 * @par Example
 * @code
 * auto &locator = ServiceLocator::instance();
 * locator.registerService<IPlatform>(&platform);
 * IPlatform *p = locator.get<IPlatform>();
 * @endcode
 */
class ServiceLocator
{
    public:
    /**
     * @brief Register a service instance.
     * @tparam T Interface type (e.g., IPlatform, IRenderer).
     * @param service Pointer to the concrete implementation.
     */
    template <typename T> void registerService(T *service)
    {
        services_[std::type_index(typeid(T))] = service;
    }

    /**
     * @brief Retrieve a registered service.
     * @tparam T Interface type.
     * @return Pointer to the service, or nullptr if not registered.
     */
    template <typename T> T *get()
    {
        auto it = services_.find(std::type_index(typeid(T)));
        if (it != services_.end())
            return static_cast<T *>(it->second);
        return nullptr;
    }

    /**
     * @brief Get the singleton instance.
     * @return Reference to the global ServiceLocator.
     */
    static ServiceLocator &instance();

    private:
    std::unordered_map<std::type_index, void *> services_;
};
